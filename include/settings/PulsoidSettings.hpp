#pragma once
#include "settings/Settings.hpp"
#include "i18n.hpp"

namespace HeartBeat {
    
    class PulsoidSettings : public Settings {
        HMUI::CurvedTextMeshPro* tokenText = nullptr;
        bool tokenTextIsDirty = false;

        UnityEngine::UI::Button *PairInBrowserBtn = nullptr, *BrowserCompleteBtn = nullptr, *CancelBrowserPairBtn = nullptr;

        HMUI::CurvedTextMeshPro* errMsgText = nullptr;


    public:
        PulsoidSettings():Settings("Pulsoid Connect", LANG->pulsoid, "<3") { }
        void CreateElements() override;
        // void Update() override;
        void Close() override;

        void SyncModConfig();

        void setButtonBusying();
        void setButtonOpeningUrl();
        void setButtonPairing();
        void setButtonPairDone();
    };

    
}