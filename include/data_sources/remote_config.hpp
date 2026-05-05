#pragma once

#include <string>

#define SERVER_HOST "http://heart.0xf7.top"
#define WS_SERVER_HOST "ws://heart.0xf7.top"

namespace HeartBeat {

std::string CheckHypeRateWebSocketIdentity();
const char* getQuestDeviceName();

} // namespace HeartBeat