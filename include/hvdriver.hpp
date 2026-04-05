#pragma once
#include "hv/EventLoop.h"

namespace HeartBeat{
    enum HVClientStatus{
        HV_UNINIT,
        HV_OPENED,
        HV_CLOSED
    };

    hv::EventLoopPtr getHvLoop();
}