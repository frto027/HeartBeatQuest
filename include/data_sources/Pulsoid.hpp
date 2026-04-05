#pragma once

#include "DataSource.hpp"
#include <memory>
#include <string>
#include <mutex>
#include "hv/WebSocketClient.h"
#include "hvdriver.hpp"

namespace HeartBeat{

class HeartBeatPulsoidDataSource:public DataSource{
    private:
        volatile int the_heart;
        volatile bool has_unread_heart_data = false;
        
        bool closed = false; // set to true to close the thread

        void CreateSocket();

        bool safe_pairing = false;

        bool safe_pair_done_wanted = false;

        std::unique_ptr<hv::WebSocketClient> hvClient = nullptr;
        HeartBeat::HVClientStatus hvClientStatus = HV_UNINIT;

        hv::TimerID safe_pair_keepalive_timer = INVALID_TIMER_ID;
        std::string pair_token_url = "";
    public:
        HeartBeatPulsoidDataSource();
        bool GetData(int& heartbeat) override;
    
        static void * ServerThread(void *self);
        
        void ResetConnection();

        void RequestPair(std::string pair_str);
        void RequestSafePair();
        void CancelSafePair(){
            safe_pairing = false;
        }
        bool IsSafePairing(){
            return safe_pairing;
        }

        void SafePairDone();


        bool modconfig_is_dirty = false;

        // a async message from service thread to main thread.
        bool url_open_wanted = false;
        std::string url;
        std::mutex url_mutex;


        bool err_message_dirty = false;
        std::string err_message;
        std::mutex err_message_mutex;
    private:

        void err(std::string message){
            std::lock_guard<std::mutex> g(err_message_mutex);
            err_message = message;
            err_message_dirty = true;
        }
};
    



}