#include "SettingsSnapshot.hpp"
#include "ModConfig.hpp"
#include "data_sources/DataSource.hpp"

HeartBeat::SettingsSnapshot::SettingsSnapshot() {
    auto& config = getModConfig();
    ModEnabled = config.Enabled.GetValue();
    DataSourceType = (HeartBeat::DataSourceType)config.DataSourceType.GetValue();
}

HeartBeat::SettingsSnapshot* HeartBeat::SettingsSnapshot::getInstance() {
    static HeartBeat::SettingsSnapshot* instance = nullptr;
    if (!instance)
        instance = new SettingsSnapshot();
    return instance;
}