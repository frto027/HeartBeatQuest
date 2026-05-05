#include "settings/Settings.hpp"
#include "SettingsSnapshot.hpp"
#include "data_sources/DataSource.hpp"
#include "settings/BleSettings.hpp"
#include "settings/HypeRateSettings.hpp"
#include "settings/MainSettings.hpp"
#include "settings/OSCSettings.hpp"
#include "settings/PulsoidSettings.hpp"
#include "UnityEngine/Application.hpp"
#include <vector>

std::vector<HeartBeat::Settings*> HeartBeat::SettingsUI::settings;
bool HeartBeat::private_ui = true;

void HeartBeat::OpenWebpage(std::string url) {
#if defined(GAME_VER_1_37_0)
    static auto UnityEngine_Application_OpenURL =
        il2cpp_utils::resolve_icall<void, StringW>("UnityEngine.Application::OpenURL");
    UnityEngine_Application_OpenURL(url);
#else
    UnityEngine::Application::OpenURL(url);
#endif
}

void HeartBeat::SettingsUI::Setup() {
    auto mainSettings = new MainSettings();
    mainSettings->Register();
    HeartBeat::SettingsUI::settings.push_back(mainSettings);


    Settings* dataSourceSettings = nullptr;

    switch (SettingsSnapshot::getInstance()->DataSourceType) {
        case HeartBeat::DataSourceType::DS_BLE:
            dataSourceSettings = new BleSettings();
            break;
        case HeartBeat::DataSourceType::DS_HypeRate:
            dataSourceSettings = new HypeRateSettings();
            break;
        case HeartBeat::DataSourceType::DS_Pulsoid:
            dataSourceSettings = new PulsoidSettings();
            break;
        case HeartBeat::DataSourceType::DS_OSC:
            dataSourceSettings = new OSCSettings();
            break;
        default:
            break;
    }
    if (dataSourceSettings) {
        dataSourceSettings->Register();
        settings.push_back(dataSourceSettings);
    }
}

void HeartBeat::SettingsUI::Update() {
    if (HeartBeat::Settings::active_setthings_ui_count == 0)
        return;

    static int slow_down = 0;
    if (slow_down++ < 20)
        return;
    slow_down = 0;
    for (auto it = SettingsUI::settings.begin(), end = SettingsUI::settings.end(); it != end; ++it) {
        if ((**it).isActive()) {
            (**it).Update();
        }
    }
}