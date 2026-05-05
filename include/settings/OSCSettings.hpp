#pragma once
#include "HMUI/CurvedTextMeshPro.hpp"
#include "settings/Settings.hpp"
#include "i18n.hpp"

namespace HeartBeat {

class OSCSettings : public Settings {
    BSML::CustomListTableData* osc_list = nullptr;
    std::vector<std::string> osc_addr;

    void UpdateOscScrollList();
    void UpdateSelectedOscValue(int idx);

public:
    OSCSettings()
        : Settings("OSC Source", LANG->heart_osc_senders, "<3") {}
    void CreateElements() override;
    void Update() override;

    HMUI::CurvedTextMeshPro* mDnsNameText = nullptr;
};


} // namespace HeartBeat