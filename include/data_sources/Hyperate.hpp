#pragma once

#include "DataSource.hpp"
#include "hv/WebSocketClient.h"
#include "hvdriver.hpp"
#include <memory>
namespace HeartBeat{


class HeartBeatHypeRateDataSource:public DataSource{
    private:
        volatile int the_heart;
        volatile bool has_unread_heart_data = false;
        
        bool closed = false; // set to true to close the thread

        void CreateSocket();

        bool resetRequest = false;


        std::unique_ptr<hv::WebSocketClient> hvClient = nullptr;
        HeartBeat::HVClientStatus hvClientStatus = HV_UNINIT;
    public:
        HeartBeatHypeRateDataSource();
        bool GetData(int& heartbeat) override;
    
        static void * ServerThread(void *self);
        
        void ResetConnection(){
            resetRequest = true;
        }

        bool has_message_from_server = false;
        char message_from_server[256];
        std::mutex message_from_server_mutex;


    private:
};


}