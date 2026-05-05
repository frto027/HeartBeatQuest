#include "ModObject.hpp"
#include "UnityEngine/GameObject.hpp"
#include "DataHub.hpp"
#include "data_sources/DataSource.hpp"
#include "java/ModHelper.h"
#include "settings/Settings.hpp"
#include <mutex>
#include "BackgroundThread.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
DEFINE_TYPE(HeartBeat, ModObject);


namespace HeartBeat {

void InitModObject() {
    static std::once_flag initModObjFlag;
    std::call_once(initModObjFlag, []() {
        auto obj = UnityEngine::GameObject::New_ctor();
        obj->set_name("HeartBeatQuestModLifeCycleObject");
        obj->AddComponent<ModObject*>();
        DataSource::getInstance()->LateStart();
    });
}

void runInUnityThread(std::function<void()> func) {
    BSML::MainThreadScheduler::Schedule(func);
}

void ModObject::Start() {
    UnityEngine::GameObject::DontDestroyOnLoad(gameObject);
}

void ModObject::Update() {
    HeartBeat::SettingsUI::Update();
    HeartBeat::DataHub::getInstance()->Update();
}


void ModObject::OnDestroy() {}

void ModObject::OnApplicationQuit() {
    if (JavaModHelper::instance) {
        JavaModHelper::instance->OnModExit();
    }

    terminateBackground();
}

} // namespace HeartBeat