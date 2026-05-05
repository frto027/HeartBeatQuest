#include "UIManager.hpp"

HeartBeat::UIManager* HeartBeat::UIManager::getInstance() {
    static UIManager* manager = nullptr;
    if (!manager)
        manager = new UIManager();
    return manager;
}