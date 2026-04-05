#include "hv/hv.h"
#include "hv/EventLoopThread.h"
#include <mutex>
#include "hvdriver.hpp"

namespace HeartBeat{
    hv::EventLoopPtr getHvLoop(){
        static hv::EventLoopThread loop_thread;
        static std::once_flag initflag;

        std::call_once(initflag, [](){
            loop_thread.start();
        });

        return loop_thread.loop();
    }
}