#include <cstddef>
#include <cstdlib>
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

#include <unistd.h>
#include "BackgroundThread.hpp"
#include "BeatLeaderRecorder.hpp"
#include "HttpClient.hpp"
#include "ModConfig.hpp"
#include "ModObject.hpp"
#include "data_sources/DataSource.hpp"
#include "ixwebsocket/IXHttp.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "main.hpp"

#include "data_sources/Pulsoid.hpp"
#include "data_sources/remote_config.hpp"
#include "UIManager.hpp"
#include "settings/Settings.hpp"


namespace HeartBeat{



HeartBeatPulsoidDataSource::HeartBeatPulsoidDataSource():DataSource(DataSourceType::DS_Pulsoid){
    {
        std::lock_guard<std::mutex> g(Recorder::heartDeviceNameLock);
        Recorder::heartDeviceName = HEART_DEV_NAME_PULSOID;
    }
}

void HeartBeatPulsoidDataSource::LateStart(){
    // setup websocket
    websocket.setUrl(WS_SERVER_HOST "/hyperate");
    ix::WebSocketHttpHeaders headers;
    headers["User-Agent"] = getModUserAgent(true);
    websocket.setExtraHeaders(headers);
    websocket.setOnMessageCallback(
        std::bind(&HeartBeatPulsoidDataSource::onWebSocketMessage, this,
                    std::placeholders::_1));

}

void HeartBeatPulsoidDataSource::ResetConnection(){
    runBackground([this](){
        websocket.stop();
        if(getModConfig().PulsoidToken.GetValue() == getModConfig().PulsoidToken.GetDefaultValue())
            return ;
        websocket.setUrl("ws://dev.pulsoid.net/api/v1/data/real_time?response_mode=text_plain_only_heart_rate&access_token=" + getModConfig().PulsoidToken.GetValue());
        websocket.start();
        getLogger().info("websocket connection opened executed");
    });
}

void HeartBeatPulsoidDataSource::onWebSocketMessage(const ix::WebSocketMessagePtr& ptr){
    if(!ptr)
        return;
    if (ptr->type == ix::WebSocketMessageType::Error)
    {
        std::stringstream ss;
        ss << "Error: "         << ptr->errorInfo.reason      << std::endl;
        ss << "#retries: "      << ptr->errorInfo.retries     << std::endl;
        ss << "Wait time(ms): " << ptr->errorInfo.wait_time   << std::endl;
        ss << "HTTP Status: "   << ptr->errorInfo.http_status << std::endl;
        getLogger().error("Websocket error: \n{}", ss.str());
        return;
    }


    if(ptr->type != ix::WebSocketMessageType::Message)
        return;
    auto & payload = ptr->str;
    if(payload.size() > 0 && payload.size() < 10){
        the_heart = atoi(payload.c_str());
        has_unread_heart_data = true;
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


void HeartBeatPulsoidDataSource::Update(){
    if(keep_alive_url.has_value() && keep_alive_total_request_count < 40){
        if(keep_alive_timer-- < 0){
            keep_alive_timer = 60 * 40;
            keep_alive_total_request_count++;
            httpGetUrl(keep_alive_url.value());
        }
    }
}

void HeartBeatPulsoidDataSource::RequestSafePair(std::function<void(void)> ondone_unity, std::function<void(std::string /* reason */)> onfail_unity){
    runBackground([this, ondone_unity, onfail_unity](){
        websocket.stop();
        getModConfig().PulsoidToken.SetValue(getModConfig().PulsoidToken.GetDefaultValue());

        getLogger().info("Start safe pair");
        httpGetUrl(SERVER_HOST "/pulsoid/safe/start", [this, ondone_unity, onfail_unity](ix::HttpResponsePtr resp){
            if(!resp){
                runInUnityThread(std::bind(onfail_unity, "Invalid HTTP response"));
                return;
            }
            if(resp->statusCode != 200){
                runInUnityThread(std::bind(onfail_unity, "Invalid HTTP response"));
                return;
            }
            std::string& pair_token = resp->body;
            if(pair_token[0] == '?'){
                runInUnityThread(std::bind(onfail_unity, pair_token.substr(1)));
                return;
            }
            if(pair_token.size() <= 0 || pair_token.size() > 80){
                runInUnityThread(std::bind(onfail_unity, "Server error, check your internet"));
                return;
            }

            
            runInUnityThread([this, pair_token, ondone_unity](){
                this->token_url = std::string(SERVER_HOST "/pulsoid/safe/token?token=") + pair_token;
                this->keep_alive_url = SERVER_HOST "/pulsoid/safe/keep_alive?token=" + pair_token;
                this->keep_alive_timer = 0;
                this->keep_alive_total_request_count=0;

                OpenWebpage(SERVER_HOST "/pulsoid/safe/redir?token=" + pair_token);
                ondone_unity();
            });
        });
    });
}

void HeartBeatPulsoidDataSource::SafePairDone(std::function<void(void)> ondone, std::function<void(void)> onpending/* user clicked done button, but actually not done */, std::function<void(std::string)> onfail){
    keep_alive_url = {};
    keep_alive_timer = 0;
    keep_alive_total_request_count = 0;
    if(this->token_url){
        runBackground([this, ondone, onfail, onpending, token_url = this->token_url.value()](){
            httpGetUrl(token_url, [this, ondone, onfail, onpending](ix::HttpResponsePtr resp){
                if(!resp){
                    onfail("Invalid response");
                    return ;
                }
                if(resp->statusCode != 200){
                    onfail("Invalid response code");
                    return;
                }
                std::string& token = resp->body;
                if(token.size() == 0){
                    onfail("Invalid token");
                    return;
                }
                if(token[0] == '?'){
                    if(token == "?authorization_pending"){
                        onpending();
                        return;
                    }
                    getLogger().error("Pair failed {}", token);
                    onfail(token.substr(1));
                    return;
                }

                if(
                    (token[0] >= 'a' && token[0] <= 'z')
                    || (token[0] >= 'A' && token[0] <= 'Z')
                    || (token[0] >= '0' && token[0] <= '9')
                    || token[0] == '-'
                ){
                    getModConfig().PulsoidToken.SetValue(token);
                    ondone();
                }else{
                    getLogger().error("Pair failed, invalid token: {}", token.c_str());
                    onfail("Invalid token");
                }
            });
        });
    }else{
        onfail("Invalid token url");
    }

}

void HeartBeatPulsoidDataSource::SafePairCancel(){
    this->token_url = {};
    this->keep_alive_url = {};
    this->keep_alive_timer = 0;
    this->keep_alive_total_request_count=0;
}

}
