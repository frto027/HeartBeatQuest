#pragma once
#include "UnityEngine/GameObject.hpp"
#include "settings/Settings.hpp"
#include "i18n.hpp"

namespace HeartBeat {

class MainSettings : public Settings {
public:
    MainSettings()
        : Settings("HeartBeatQuest Main Config", LANG->heart_config, "<3") {}
    void CreateElements() override;

    UnityEngine::UI::Button* private_public_btn = nullptr;

    void UpdateContent();
};


} // namespace HeartBeat