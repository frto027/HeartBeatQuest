#pragma once

#include "config-utils/shared/config-utils.hpp"
#include <string>

// clang-format off

#if defined(GAME_VER_1_28_0) || defined(GAME_VER_1_35_0) || defined(GAME_VER_1_37_0)
DECLARE_CONFIG(ModConfig,
#else
DECLARE_CONFIG(ModConfig){
#endif
    // Declare "VariableA"
    //CONFIG_VALUE(VariableA, std::string, "Variable Name", "Variable Value");

    CONFIG_VALUE(Enabled, bool, "Enabled", true);

    CONFIG_VALUE(EnableRecord, bool, "Enable Record", true);
    CONFIG_VALUE(RecordDevName, bool, "Record Device Name", true);

    CONFIG_VALUE(SelectedBleMac, std::string, "Selected BLE Device Mac", "");

    CONFIG_VALUE(SelectedUI, std::string, "SelectedUIBundle", "Default");

    CONFIG_VALUE(ModLang, std::string, "Mod Language", "auto");

    CONFIG_VALUE(DisplayEnergy, bool, "Display Energy", true);

    CONFIG_VALUE(Age, int, "Age", 25);
    CONFIG_VALUE(MaxHeart, int, "MaxHeart", 220 - 25);

    CONFIG_VALUE(DataSourceType, int, "DataSourceType", 2);

    CONFIG_VALUE(OSCPort, int, "OSC Port", 9000);
    CONFIG_VALUE(OSCSelectedDevice, std::string, "OSC Selected Device", "");

    CONFIG_VALUE(OSC_MDNS_ENABLED, bool, "Enable MDns for OSC", false);
    CONFIG_VALUE(OSC_MDNS_NAME, std::string, "MDns name for osc", "osc.heartbeatquest.local");

    CONFIG_VALUE(HypeRateId, std::string, "HypeRateID", "")

    CONFIG_VALUE(PulsoidToken, std::string, "PulsoidToken", "00000000-0000-0000-0000-000000000000")

    CONFIG_VALUE(HypeRateWebSocketIdentity, std::string, "HypeRateWebsocketIdentity", "")

    CONFIG_VALUE(IgnoreQounters, bool, "IgnoreQounters", false)
#if defined(GAME_VER_1_28_0) || defined(GAME_VER_1_35_0) || defined(GAME_VER_1_37_0)
);
#else
};
#endif

// clang-format on
