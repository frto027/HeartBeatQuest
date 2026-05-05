#include "UnityEngine/Time.hpp"

#include "../shared/HeartBeatApi.h"
#include "BeatLeaderRecorder.hpp"
#include "data_sources/DataSource.hpp"

#include <cstddef>
#include "DataHub.hpp"
namespace HeartBeat {

void DataHub::Update() {
    int curFrame = UnityEngine::Time::get_frameCount();
    if (curFrame == lastUpdateFrame)
        return;
    lastUpdateFrame = curFrame;

    // Update heart rate data from data sources or replay file
    if (HeartBeat::Recorder::isReplaying()) {
        hasNewData = HeartBeat::Recorder::ReplayGetData(data);
    } else {
        auto instance = HeartBeat::DataSource::getInstance();
        instance->Update();
        hasNewData = instance->GetData(data);

        // we are obtain data from physical heart rate device

        if (hasNewData && HeartBeat::IsDatasourceAbleToRecord()) {
            HeartBeat::Recorder::RecordDataIfNeeded(data);
        }
    }
}

bool DataHub::GetData(int& heartrate) {
    heartrate = data;
    return hasNewData;
}

DataHub* DataHub::getInstance() {
    static DataHub* instance = nullptr;
    if (!instance)
        instance = new DataHub();
    return instance;
}
} // namespace HeartBeat
