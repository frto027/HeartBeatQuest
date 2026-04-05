#include <atomic>
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
#include "hv/HttpMessage.h"
#include "hv/hloop.h"
#include "i18n.hpp"
#include "main.hpp"

#include <sys/system_properties.h>
#include "data_sources/Hyperate.hpp"

#include "data_sources/remote_config.hpp"
#include "UIManager.hpp"

#include "hvdriver.hpp"
#include "hv/WebSocketClient.h"

/*

You know you won't copy these code to get heart rate in other project
because it uses a private server.
If you have similar needs, please contact HypeRate official, they are kind people. heart. :) 

*/
namespace HeartBeat{

HeartBeatHypeRateDataSource::HeartBeatHypeRateDataSource():DataSource(DataSourceType::DS_HypeRate){
    {
        std::lock_guard<std::mutex> g(Recorder::heartDeviceNameLock);
        Recorder::heartDeviceName = HEART_DEV_NAME_HYPERATE;
    }
    this->CreateSocket();
}

static time_t last_ping_time = 0;
static bool con_opened = false;


static int failed_count = 0;

inline int retry_sleep_time(){
    if(failed_count < 30)
        return 3;
    return 10;
}

std::string CheckHypeRateWebSocketIdentity(){
    std::string ret = getModConfig().HypeRateWebSocketIdentity.GetValue();
    if(ret == ""){
        char buff[33];
        FILE * f = fopen("/dev/urandom", "rb");
        bool handled = false;
        const char * avaliable_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()";
        int char_len = strlen(avaliable_chars);
        if(f){
            getLogger().info("HypeRate Websocket random identity generrated from /dev/urandom");
            uint8_t numbers[32];
            if(32 == fread(numbers, 1, 32, f)){
                handled = true;
                for(int i=0;i<32;i++){
                    buff[i] = avaliable_chars[numbers[i] % char_len];
                }
                buff[32] = '\0';
            }
        }

        if(f){
            fclose(f);
            f = NULL;
        }

        if(!handled){
            getLogger().warn("HypeRate Websocket random identity not generated, fallback to random call");
            for(int i=0;i<32;i++){
                buff[i] = avaliable_chars[random() % char_len];
            }
            buff[32] = '\0';
        }

        getModConfig().HypeRateWebSocketIdentity.SetValue(buff);
        ret = buff;
    }
    return ret;
}

const char * getQuestDeviceName(){
    static char model_string[PROP_VALUE_MAX+1] = "unk";
    __system_property_get("ro.product.model", model_string);
    return model_string;
}

static std::function<void(std::error_code)> timer_impl;
static int current_retry_time_already = 0;
void HeartBeatHypeRateDataSource::CreateSocket(){

    if(!this->hvClient){

        HeartBeat::getHvLoop()->setTimerInLoop(1000, [this](hv::TimerID){

            if(this->resetRequest){
                this->resetRequest = false;
                if(this->hvClient){
                    this->hvClient->close();
                    this->hvClientStatus = HV_CLOSED;
                }
            }

            if(UIManager::getInstance()->hasReader() && getModConfig().HypeRateId.GetValue().length() > 0){
                if( !this->closed && (this->hvClientStatus == HV_UNINIT || hvClientStatus == HV_CLOSED)){
                    http_headers mod_header = DefaultHeaders;
                    {
                        char ua_buff[1024];
                        std::string identity = CheckHypeRateWebSocketIdentity();
                        sprintf(ua_buff, "%s %s %s", "HeartBeatQuest/" VERSION " BeatSaber/" GAME_VERSION, identity.c_str(), getQuestDeviceName());
                        mod_header["UserAgent"] = ua_buff;
                    }
                    this->hvClientStatus = HV_OPENED;

                    reconn_setting_t reconn;
                    reconn_setting_init(&reconn);
                    reconn.min_delay = 1000 * 2;
                    reconn.max_delay = 1000 * 60;
                    reconn.delay_policy = 2;
                    this->hvClient->setReconnect(&reconn);

                    this->hvClient->open(WS_SERVER_HOST "/hyperate", mod_header);
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
            if(payload.length() > 1 && payload[0] == 'S'){
                const char * json_str = payload.c_str() + 1;
                rapidjson::Document d;
                d.Parse(json_str);
                if(!d.IsObject())
                    return;
                auto type_it = d.FindMember("type");
                if(type_it == d.MemberEnd())
                    return;
                if(!type_it->value.IsString())
                    return;
                std::string type = type_it->value.GetString();
                if(type == "message"){
                    auto msg_it = d.FindMember("msg");
                    auto actions_it = d.FindMember("actions");
                    std::vector<std::string> actions;

                    if(msg_it != d.MemberEnd() && msg_it->value.IsString()){
                        size_t len = msg_it->value.GetStringLength();
                        std::lock_guard<std::mutex> g(this->message_from_server_mutex);


                        if(len + 10 >= sizeof(this->message_from_server)){
                            size_t copy_len = sizeof(this->message_from_server) - 10;

                            memcpy(this->message_from_server, msg_it->value.GetString(), copy_len);
                            this->message_from_server[copy_len] = '.';
                            this->message_from_server[copy_len+1] = '.';
                            this->message_from_server[copy_len+2] = '.';
                            this->message_from_server[copy_len+3] = '\0';
                        }else{
                            memcpy(this->message_from_server, msg_it->value.GetString(), len);
                            this->message_from_server[len] = '\0';
                        }
                        this->has_message_from_server = true;
                    }else{
                        std::lock_guard<std::mutex> g(this->message_from_server_mutex);
                        strcpy(this->message_from_server, "invalid server message");
                    }

                    if(actions_it != d.MemberEnd() && actions_it->value.IsArray()){
                        for(auto & e : actions_it->value.GetArray()){
                            if(e.IsString()){
                                const char * action = e.GetString();
                                //do the action here
                                if(strcmp(action, "close") == 0){
                                    closed = true;
                                    this->hvClient->close();
                                }

                                if(strcmp(action, "reset") == 0){
                                    getModConfig().HypeRateWebSocketIdentity.SetValue("");
                                }
                            }
                        }
                    }
                }
                return;
            }

            {
                const char * json_str = payload.c_str();
                rapidjson::Document d;
                d.Parse(json_str);
                if(!d.IsObject())
                    return;

                auto it = d.FindMember("payload");
                if(it == d.MemberEnd())
                    return;
                auto & payload = it->value;

                if(!payload.IsObject())
                    return;
                auto hr_it = payload.FindMember("hr");
                if(hr_it == payload.MemberEnd())
                    return;
                if(!hr_it->value.IsInt())
                    return;
                int heart = hr_it->value.GetInt();
                std::atomic_thread_fence(std::memory_order_acquire);
                this->the_heart = heart;
                std::atomic_thread_fence(std::memory_order_acquire);
                this->has_unread_heart_data = true;
                std::atomic_thread_fence(std::memory_order_acquire);
            }
        };
    }
    
}

bool HeartBeatHypeRateDataSource::GetData(int&heartbeat){
    if(has_unread_heart_data)
    {
        has_unread_heart_data = false;
        heartbeat = the_heart;
        return true;
    }
    return false;
}

}
