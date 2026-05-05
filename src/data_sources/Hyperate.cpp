#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <jni.h>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/endian.h>
#include <sys/socket.h>
#include <sys/types.h>


#include "BackgroundThread.hpp"
#include "BeatLeaderRecorder.hpp"
#include "HttpClient.hpp"
#include "ModConfig.hpp"
#include "ModObject.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "i18n.hpp"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketHttpHeaders.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "main.hpp"
#include <unistd.h>


#include "data_sources/Hyperate.hpp"
#include <sys/system_properties.h>


#include "UIManager.hpp"
#include "data_sources/remote_config.hpp"
#include "settings/PreviewObj.hpp"


/*

You know you won't copy these code to get heart rate in other project
because it uses a private server.
If you have similar needs, please contact HypeRate official, they are kind
people. heart. :)

*/
namespace HeartBeat {

void HeartBeatHypeRateDataSource::Update() {
    static int counter = 0;
    counter++;
    if (counter % (60 * 20) == 0) {
        // check the socket connection
        auto state = websocket.getReadyState();
        if (!closed && state == ix::ReadyState::Closed) {
            // we need connect to socket
            if (UIManager::getInstance()->hasReader())
                RestartSocket();
        }
    }
}

void HeartBeatHypeRateDataSource::OnNewReader() {
    auto state = websocket.getReadyState();
    if (!closed && state == ix::ReadyState::Closed) {
        RestartSocket();
    }
}

HeartBeatHypeRateDataSource::HeartBeatHypeRateDataSource()
    : DataSource(DataSourceType::DS_HypeRate)
    , status("") {
    Recorder::SetHeartDeviceName(HEART_DEV_NAME_HYPERATE);
}

void HeartBeatHypeRateDataSource::LateStart() {
    // setup websocket
    websocket.setUrl(WS_SERVER_HOST "/hyperate");
    websocket.setPingInterval(15);
    ix::WebSocketHttpHeaders headers;
    headers["User-Agent"] = getModUserAgent();
    websocket.setExtraHeaders(headers);
    websocket.setOnMessageCallback(
        std::bind(&HeartBeatHypeRateDataSource::onWebSocketMessage, this, std::placeholders::_1));
    websocket.enableAutomaticReconnection();
}

void HeartBeatHypeRateDataSource::RestartSocket(std::optional<std::function<void(void)>> callback) {
    runBackground([this, callback = std::move(callback)]() {
        websocket.stop();

        if (closed || getModConfig().HypeRateId.GetValue() == "") {
            if (closed) {
                status = LANG->hyperate_refused;
            } else {
                status = LANG->hyperate_no_id;
            }
            if (callback.has_value())
                runInUnityThread(std::move(callback.value()));
            return;
        }

        websocket.start();
        status = LANG->hyperate_con_start;
        if (callback.has_value()) {
            runInUnityThread(std::move(callback.value()));
        }
    });
}

// call thread: websocket thread
void HeartBeatHypeRateDataSource::onWebSocketMessage(const ix::WebSocketMessagePtr& ptr) {
    // getLogger().info("Received message {}", (int)ptr->type);

    if (ptr->type == ix::WebSocketMessageType::Open) {
        getLogger().info("websocket connection opened, send start message");
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
        // getLogger().info("Send package to server: {}", toSend);

        websocket.send(toSend);
        runInUnityThread([this]() { status = LANG->hyperate_connected; });
        return;
    }

    if (ptr->type == ix::WebSocketMessageType::Error) {
        std::stringstream ss;
        ss << "Error: " << ptr->errorInfo.reason << std::endl;
        ss << "#retries: " << ptr->errorInfo.retries << std::endl;
        ss << "Wait time(ms): " << ptr->errorInfo.wait_time << std::endl;
        ss << "HTTP Status: " << ptr->errorInfo.http_status << std::endl;
        getLogger().error("Websocket error: \n{}", ss.str());

        runInUnityThread([this, reason = ptr->errorInfo.reason, retry = ptr->errorInfo.retries]() {
            std::stringstream ss;
            ss << LANG->hyperate_network_error << reason;
            if (retry > 0) {
                ss << "\n" << LANG->hyperate_retry << "(" << retry << ")";
            }
            status = ss.str();
        });
        return;
    }


    if (ptr->type == ix::WebSocketMessageType::Message) {
        const std::string& payload = ptr->str;

        if (payload.length() > 1 && payload[0] == 'S') {
            const char* json_str = payload.c_str() + 1;
            rapidjson::Document d;
            d.Parse(json_str);
            if (!d.IsObject())
                return;
            auto type_it = d.FindMember("type");
            if (type_it == d.MemberEnd())
                return;
            if (!type_it->value.IsString())
                return;
            std::string type = type_it->value.GetString();
            handleServerPayload(type, d);

            return;
        }

        {
            const char* json_str = payload.c_str();
            rapidjson::Document d;
            d.Parse(json_str);
            if (!d.IsObject())
                return;
            handleHyperatePaylod(d);
        }
    }
}

void HeartBeatHypeRateDataSource::handleServerPayload(const std::string& type, rapidjson::Document& d) {
    if (type == "message") {
        auto msg_it = d.FindMember("msg");
        auto actions_it = d.FindMember("actions");
        std::vector<std::string> actions;

        if (msg_it != d.MemberEnd() && msg_it->value.IsString()) {
            size_t len = msg_it->value.GetStringLength();

            std::string msg = msg_it->value.GetString();
            if (msg.length() > 255) {
                msg = msg.substr(0, 255) + ".....";
            }
            runInUnityThread([this, msg = std::move(msg)]() {
                status = LANG->hyperate_you_have_message;
                serverMessage = msg;
            });
        } else {
            runInUnityThread([this]() { status = "Error: invalid server message"; });
        }

        if (actions_it != d.MemberEnd() && actions_it->value.IsArray()) {
            for (auto& e : actions_it->value.GetArray()) {
                if (e.IsString()) {
                    const char* action = e.GetString();
                    // do the action here
                    if (strcmp(action, "close") == 0) {
                        closed = true;
                        runBackground([this]() { websocket.stop(); });
                    }

                    if (strcmp(action, "reset") == 0) {
                        getModConfig().HypeRateWebSocketIdentity.SetValue("");
                    }
                }
            }
        }
    }
}

void HeartBeatHypeRateDataSource::handleHyperatePaylod(rapidjson::Document& d) {
    auto it = d.FindMember("payload");
    if (it == d.MemberEnd())
        return;
    auto& payload = it->value;

    if (!payload.IsObject())
        return;
    auto hr_it = payload.FindMember("hr");
    if (hr_it == payload.MemberEnd())
        return;
    if (!hr_it->value.IsInt())
        return;
    this->the_heart = hr_it->value.GetInt();
    this->has_unread_heart_data = true;
}

bool HeartBeatHypeRateDataSource::GetData(int& heartbeat) {
    if (has_unread_heart_data) {
        has_unread_heart_data = false;
        heartbeat = the_heart;
        return true;
    }
    return false;
}

} // namespace HeartBeat
