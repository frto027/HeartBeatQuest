#pragma once
#include "settings/Settings.hpp"
#include "i18n.hpp"

namespace HeartBeat {
    
    class HypeRateSettings : public Settings {
    public:
        HypeRateSettings():Settings("HypeRate Connect", LANG->hyperate, "<3") { }
        void CreateElements() override;

        std::string hyperate_id;

    private:
        std::vector<UnityEngine::UI::Button*> buttons;
        void disableBtns();
        void enableBtns();
    };

    
}