#pragma once

#include "UnityEngine/GameObject.hpp"
#include "HeartBeat.hpp"
namespace HeartBeat {
struct MainMenuPreviewer {
    MainMenuPreviewer();

    UnityEngine::GameObject* MainMenuPreviewObject = nullptr;
    HeartBeat::HeartBeatObj* MainMenuPreviewObjectComp = nullptr;

    static MainMenuPreviewer* getInstance();

    void Show();
    void Hide();
    void Reload();
};

} // namespace HeartBeat