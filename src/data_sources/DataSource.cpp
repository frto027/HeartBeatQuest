#include "data_sources/DataSource.hpp"
#include "ModConfig.hpp"
#include "data_sources/Bluetooth.hpp"
#include "data_sources/Hyperate.hpp"
#include "data_sources/Pulsoid.hpp"
#include "data_sources/OSC.hpp"
#include "data_sources/Random.hpp"
#include "SettingsSnapshot.hpp"

namespace HeartBeat {
DataSourceType dataSourceType = DS_RANDOM;

DataSource* DataSource::getInstance() {
    static DataSource* instance = nullptr;
    if (instance == nullptr) {
        HeartBeat::dataSourceType = (HeartBeat::DataSourceType)getModConfig().DataSourceType.GetValue();

        switch (HeartBeat::dataSourceType) {
            case DS_RANDOM:
                instance = new RandomDataSource();
                break;
            case DS_LAN:
                instance = new RandomDataSource();
                break;
            case DS_BLE:
                instance = new HeartBeatBleDataSource();
                break;
            case DS_OSC:
                instance = new HeartBeatOSCDataSource();
                break;
            case DS_HypeRate:
                instance = new HeartBeatHypeRateDataSource();
                break;
            case DS_Pulsoid:
                instance = new HeartBeatPulsoidDataSource();
                break;
        }
    }
    return instance;
}

bool DataSource::GetData(int& heartbeat) {
    return false;
}
} // namespace HeartBeat

bool HeartBeat::IsDatasourceAbleToRecord() {
    switch (SettingsSnapshot::getInstance()->DataSourceType) {
        case HeartBeat::DataSourceType::DS_BLE:
        case HeartBeat::DataSourceType::DS_HypeRate:
        case HeartBeat::DataSourceType::DS_Pulsoid:
        case HeartBeat::DataSourceType::DS_OSC:
            return true;
        default:
            return false;
    }
}
