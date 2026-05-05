#pragma once

#include "data_sources/DataSource.hpp"
namespace HeartBeat {
struct SettingsSnapshot {
    SettingsSnapshot();

    bool ModEnabled;
    HeartBeat::DataSourceType DataSourceType;

    static SettingsSnapshot* getInstance();
};
} // namespace HeartBeat