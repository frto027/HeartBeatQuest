#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <jni.h>
#include <memory>
#include <mutex>
#include <stdlib.h>
#include <string>
#include <sys/endian.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <system_error>
#include <unistd.h>
#include "BeatLeaderRecorder.hpp"
#include "ModConfig.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "data_sources/DataSource.hpp"
#include "hvdriver.hpp"
#include "i18n.hpp"
#include "main.hpp"

#include "hv/requests.h"
#include "hv/WebSocketClient.h"

#include "data_sources/Pulsoid.hpp"
#include "data_sources/remote_config.hpp"
#include "UIManager.hpp"


namespace HeartBeat{



HeartBeatPulsoidDataSource::HeartBeatPulsoidDataSource():DataSource(DataSourceType::DS_Pulsoid){
    {
        std::lock_guard<std::mutex> g(Recorder::heartDeviceNameLock);
        Recorder::heartDeviceName = HEART_DEV_NAME_PULSOID;
    }
    this->CreateSocket();
}

void HeartBeatPulsoidDataSource::CreateSocket(){



    if(!this->hvClient){

        HeartBeat::getHvLoop()->setTimerInLoop(1000, [this](hv::TimerID){
            if(UIManager::getInstance()->hasReader() && getModConfig().HypeRateId.GetValue().length() > 0){
                if( !this->closed && (this->hvClientStatus == HV_UNINIT || hvClientStatus == HV_CLOSED)){
                    http_headers mod_header = DefaultHeaders;
                    {
                        char buff[1024] = "";
                        sprintf(buff, "%s %s", "HeartBeatQuest/" VERSION " BeatSaber/" GAME_VERSION, getQuestDeviceName());
                        mod_header["UserAgent"] = buff;
                    }
                    this->hvClientStatus = HV_OPENED;

                    reconn_setting_t reconn;
                    reconn_setting_init(&reconn);
                    reconn.min_delay = 1000 * 2;
                    reconn.max_delay = 1000 * 60;
                    reconn.delay_policy = 2;
                    this->hvClient->setReconnect(&reconn);

                    std::string url = "ws://dev.pulsoid.net/api/v1/data/real_time?response_mode=text_plain_only_heart_rate&access_token=" + getModConfig().PulsoidToken.GetValue();

                    this->hvClient->open(url.c_str(), mod_header);
                }
            }
        });

        this->hvClient = std::unique_ptr<hv::WebSocketClient>(new hv::WebSocketClient(HeartBeat::getHvLoop()));

        this->hvClient->onopen = [this](){
            if(!this->hvClient) return;
            if(this->closed) return;

            this->hvClient->setReconnect(NULL);

            getLogger().info("websocket connection opened executed");
            std::string id = getModConfig().HypeRateId.GetValue();
            // id = "internal-testing";
            rapidjson::Document dom;
            dom.SetObject();
            dom.AddMember("_id", CheckHypeRateWebSocketIdentity(), dom.GetAllocator());
            dom.AddMember("id", id, dom.GetAllocator());
            dom.AddMember("lang", rapidjson::StringRef(LANG->lang_name), dom.GetAllocator());
            dom.AddMember("ver", VERSION, dom.GetAllocator());
            dom.AddMember("forgame", GAME_VERSION, dom.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            dom.Accept(writer);

            std::string toSend = std::string("C") + buffer.GetString();
            //getLogger().info("Send package to server: {}", toSend);

            // the return value of send function is undocumented. so we can't use it.
            this->hvClient->send(toSend);
        };

        this->hvClient->onclose = [this]() {
            hvClientStatus = HV_CLOSED;
        };
        this->hvClient->onmessage = [this](const std::string& payload){
            if(!this->hvClient) return;
            if(this->closed) return;
            //getLogger().info("{}", payload);
            //TODO
        };
    }
}


bool HeartBeatPulsoidDataSource::GetData(int&heartbeat){
    if(has_unread_heart_data)
    {
        has_unread_heart_data = false;
        heartbeat = the_heart;
        return true;
    }
    return false;
}

void HeartBeatPulsoidDataSource::ResetConnection(){
    if(this->hvClient){
        this->hvClient->close();
        this->hvClientStatus = HV_CLOSED;
    }
}

http_headers getPulsoidRequestHeader(){
    std::string ua = "HBQ/" VERSION " BS/" GAME_VERSION " " + std::string(LANG->lang_name) + " " + CheckHypeRateWebSocketIdentity();

    http_headers header;
    header["UserAgent"] = ua;
    return header;
}

void HeartBeatPulsoidDataSource::RequestSafePair(){    
    ResetConnection();

    getHvLoop()->runInLoop([this](){
        getLogger().info("Start safe pair");
        safe_pair_done_wanted = false;

        http_headers header = getPulsoidRequestHeader();

        std::string pair_token, header_string;
        auto resp = requests::get(SERVER_HOST "/pulsoid/safe/start", header);
        if(resp == NULL || resp->status_code != http_status::HTTP_STATUS_OK){
            //TODO: failed
            safe_pairing = false;
            return ;
        }
        pair_token = resp->Body();

        if(pair_token.size() > 0 && pair_token.size() < 80){
            if(pair_token[0] == '?'){
                safe_pairing = false;
                err(pair_token.c_str() + 1);
                return;
            }
            err("");
            //continue
            // auto login_url = SERVER_HOST "/pulsoid/safe/redir?token=" + pair_token;
            auto login_url = SERVER_HOST "/pulsoid/safe/redir?token=" + pair_token;

            {
                //we will open the url in the setthings thread
                std::lock_guard<std::mutex> g(this->url_mutex);
                this->url = login_url;
                this->url_open_wanted = true;

                getLogger().info("open url {}", login_url);
            }

            //get token from server
            this->pair_token_url = std::string(SERVER_HOST "/pulsoid/safe/token?token=") + pair_token;

            safe_pair_keepalive_timer = getHvLoop()->setTimerInLoop(40 * 1000, [this, pair_token, header](hv::TimerID timerId){
                if(safe_pairing == false){
                    getHvLoop()->killTimer(timerId);
                    safe_pair_keepalive_timer = INVALID_TIMER_ID;
                    return ;
                }

                auto keep_alive_url = std::string(SERVER_HOST "/pulsoid/safe/keep_alive?token=") + pair_token;
                auto resp = requests::get(keep_alive_url.c_str(), header);
            }, 60);
        }else{
            err("Server error, check your Internet.");
            safe_pairing = false;
        }

    });
}

void HeartBeatPulsoidDataSource::SafePairDone(){
    if(safe_pair_keepalive_timer != INVALID_TIMER_ID){
        getHvLoop()->killTimer(safe_pair_keepalive_timer);
        safe_pair_keepalive_timer = INVALID_TIMER_ID;
    }
    if(pair_token_url != ""){
        getHvLoop()->runInLoop([this](){
            auto resp = requests::get(pair_token_url.c_str(), getPulsoidRequestHeader());
            if(resp && resp->status_code == http_status::HTTP_STATUS_OK){
                std::string token = resp->Body();
                    if(token.size() > 0){
                        if(token[0] == '?'){
                            if(token == "?authorization_pending")
                                return ;
                            getLogger().error("Pair failed: {}", token.c_str());
                            err(token);
                            safe_pairing = false;
                            return;
                        }
                        if(
                            (token[0] >= 'a' && token[0] <= 'z')
                            || (token[0] >= 'A' && token[0] <= 'Z')
                            || (token[0] >= '0' && token[0] <= '9')
                            || token[0] == '-'
                        ){
                            getModConfig().PulsoidToken.SetValue(token);
                            safe_pairing = false;
                            modconfig_is_dirty = true;
                            err("");//succees
                        }else{
                            err("Invalid token.");
                            getLogger().error("Pair failed, invalid token: {}", token.c_str());
                            safe_pairing = false;
                        }
                    }
            }
        });
    }
}

}
