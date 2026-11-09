#pragma once
#include "HMUI/CurvedTextMeshPro.hpp"
#include "settings/Settings.hpp"
#include "SSL10nGenerated.hpp"

namespace HeartBeat {

class PulsoidSettings : public Settings {
    HMUI::CurvedTextMeshPro *tokenText = nullptr;
    bool tokenTextIsDirty = false;

    UnityEngine::UI::Button *PairInBrowserBtn = nullptr, *BrowserCompleteBtn = nullptr, *CancelBrowserPairBtn = nullptr;

    HMUI::CurvedTextMeshPro *errMsgText = nullptr;

    HMUI::CurvedTextMeshPro *statusText = nullptr;

  public:
    PulsoidSettings()
        : Settings("HEART_BEAT_QUEST_SETTHINGS_PULSOID_TITLE", "HEART_BEAT_QUEST_SETTHINGS_PULSOID_BUTTON", "<3") {}
    void CreateElements() override;
    // void Update() override;
    void Close() override;

    void SyncModConfig();

    void setButtonBusying();
    void setButtonOpeningUrl();
    void setButtonPairing();
    void setButtonPairDone();

    void Update() override;
};

} // namespace HeartBeat