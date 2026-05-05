#pragma once
#include "HMUI/CurvedTextMeshPro.hpp"
#include "settings/Settings.hpp"
#include "i18n.hpp"

namespace HeartBeat {

class HypeRateSettings : public Settings {
public:
    HypeRateSettings()
        : Settings("HypeRate Connect", LANG->hyperate, "<3") {}
    void CreateElements() override;

    std::string hyperate_id;

    void Update() override;

private:
    std::vector<UnityEngine::UI::Button*> buttons;
    void disableBtns();
    void enableBtns();

    HMUI::CurvedTextMeshPro* statusText;
    HMUI::CurvedTextMeshPro* serverMessageText;
};


} // namespace HeartBeat