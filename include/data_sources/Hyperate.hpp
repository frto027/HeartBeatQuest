#pragma once

#include "DataSource.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "ixwebsocket/IXWebSocket.h"
#include <functional>
#include <optional>

namespace HeartBeat{

class HeartBeatHypeRateDataSource:public DataSource{
    private:
        volatile int the_heart;
        volatile bool has_unread_heart_data = false;
        
        bool closed = false; // set to true to close the thread

        void CreateSocket();

        bool resetRequest = false;

        // this member should only be used in background thread
        ix::WebSocket websocket;
    public:
        HeartBeatHypeRateDataSource();
        bool GetData(int& heartbeat) override;
            
        void SetHyperateID(std::string id);
        void RestartSocket(std::optional<std::function<void(void)>> callback_unity = {});

        void Update() override;
        void LateStart() override;
    private:
        // execute in websocket thread
        void onWebSocketMessage(const ix::WebSocketMessagePtr& ptr);
        void handleServerPayload(const std::string & type, rapidjson::Document & d);
        void handleHyperatePaylod(rapidjson::Document & d);
};


}