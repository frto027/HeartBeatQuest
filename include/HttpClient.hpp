#pragma once
#include "ixwebsocket/IXHttp.h"
#include "ixwebsocket/IXHttpClient.h"
#include <functional>
#include <optional>

namespace HeartBeat {
std::string CheckHypeRateWebSocketIdentity();
std::string getModUserAgent(bool with_identity = true);
ix::HttpClient& getHttpClient();
void httpGetUrl(std::string url, std::optional<std::function<void(ix::HttpResponsePtr)>> callback = {});
} // namespace HeartBeat