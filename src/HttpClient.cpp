#include "HttpClient.hpp"
#include "BackgroundThread.hpp"
#include "ModConfig.hpp"
#include "ModObject.hpp"
#include "i18n.hpp"
#include "ixwebsocket/IXHttp.h"
#include "ixwebsocket/IXHttpClient.h"
#include "main.hpp"
#include <mutex>
#include <sys/system_properties.h>


const char* getQuestDeviceName() {
    static char model_string[PROP_VALUE_MAX + 1] = "unk";
    __system_property_get("ro.product.model", model_string);
    return model_string;
}

std::string HeartBeat::CheckHypeRateWebSocketIdentity() {
    std::string ret = getModConfig().HypeRateWebSocketIdentity.GetValue();
    if (ret == "") {
        char buff[33];
        FILE* f = fopen("/dev/urandom", "rb");
        bool handled = false;
        const char* avaliable_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()";
        int char_len = strlen(avaliable_chars);
        if (f) {
            getLogger().info("HypeRate Websocket random identity generrated from /dev/urandom");
            uint8_t numbers[32];
            if (32 == fread(numbers, 1, 32, f)) {
                handled = true;
                for (int i = 0; i < 32; i++) {
                    buff[i] = avaliable_chars[numbers[i] % char_len];
                }
                buff[32] = '\0';
            }
        }

        if (f) {
            fclose(f);
            f = NULL;
        }

        if (!handled) {
            getLogger().warn("HypeRate Websocket random identity not generated, "
                             "fallback to random call");
            for (int i = 0; i < 32; i++) {
                buff[i] = avaliable_chars[random() % char_len];
            }
            buff[32] = '\0';
        }

        getModConfig().HypeRateWebSocketIdentity.SetValue(buff);
        ret = buff;
    }
    return ret;
}


std::string HeartBeat::getModUserAgent(bool with_identity) {
    std::string identity = CheckHypeRateWebSocketIdentity();

    std::stringstream ss;
    ss << "HeartBeatQuest/" VERSION << " "   // product
       << "(BeatSaber/" GAME_VERSION << ") " // (system-information)
       << getQuestDeviceName()               // platform
        ;

    if (with_identity)
        ss << " id/" << identity;
    ss << " lang/" << (LANG ? LANG->lang_name : "?");
    return ss.str();
}

ix::HttpClient& HeartBeat::getHttpClient() {
    static ix::HttpClient client;
    // std::once_flag client_init;
    // std::call_once(client_init, [](){

    // });
    return client;
}

void HeartBeat::httpGetUrl(std::string url, std::optional<std::function<void(ix::HttpResponsePtr)>> callback) {
    runBackground([url = std::move(url), callback]() {
        auto& client = getHttpClient();
        ix::HttpRequestArgsPtr args = client.createRequest();
        args->extraHeaders["User-Agent"] = getModUserAgent();
        args->connectTimeout = 15;
        args->logger = [](const std::string& msg) { getLogger().info("{}", msg); };

        ix::HttpResponsePtr out = client.get(url, args);
        if (callback.has_value()) {
            callback.value()(out);
        }
    });
}