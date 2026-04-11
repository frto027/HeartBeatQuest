#pragma once

#include "DataSource.hpp"
#include "ixwebsocket/IXWebSocket.h"
#include <memory>
#include <optional>
#include <string>
#include <mutex>

namespace HeartBeat{

class HeartBeatPulsoidDataSource:public DataSource{
    private:
        volatile int the_heart;
        volatile bool has_unread_heart_data = false;
        
        bool closed = false; // set to true to close the thread

        ix::WebSocket websocket;

        // only operate these in unity thread
        int keep_alive_total_request_count = 0;
        int keep_alive_timer = 0;
        std::optional<std::string> keep_alive_url = {};
        std::optional<std::string> token_url = {};
    public:
        HeartBeatPulsoidDataSource();
        bool GetData(int& heartbeat) override;

        void Update() override;

        void ResetConnection();

        // call this function will request a url and open it in browser, then call ondone_unity
        void RequestSafePair(std::function<void(void)> ondone_unity, std::function<void(std::string /* reason */)> onfail_unity);
        void SafePairDone(std::function<void(void)> ondone, std::function<void(void)> onpending/* user clicked done button, but actually not done */, std::function<void(std::string)> onfail);
        void SafePairCancel();

        void LateStart() override;
    private:
        // execute in websocket thread
        void onWebSocketMessage(const ix::WebSocketMessagePtr& ptr);
};
    



}