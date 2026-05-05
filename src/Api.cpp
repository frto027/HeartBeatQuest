#include "DataHub.hpp"
#include "HeartBeatApi.h"
#include "main.hpp"

static void Update() {
    HeartBeat::DataHub::getInstance()->Update();
}

static int GetData(int* heartbeat) {
    int data;
    auto ret = HeartBeat::DataHub::getInstance()->GetData(data);
    if (heartbeat) {
        *heartbeat = data;
    }
    return ret;
}

extern "C" HeartBeatApi heartBeatApi = {
    .ApiVersion = 1,
    .Update = Update,
    .GetData = GetData,
    .SetAlternateDataUpdater =
        [](auto t) {
            getLogger().warn("HeartBeatQuest's api SetAlternateDataUpdater has been deprecated. Contact @frto027 at "
                             "BSMG's discord server if you need this.");
        },
    .__not_used2__ = {NULL},
};
