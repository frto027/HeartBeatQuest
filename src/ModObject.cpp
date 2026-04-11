#include "ModObject.hpp"
#include "UnityEngine/GameObject.hpp"
#include "DataHub.hpp"
#include "data_sources/DataSource.hpp"
#include "settings/Settings.hpp"
#include <mutex>
#include <queue>
#include "SettingsSnapshot.hpp"
#include "BackgroundThread.hpp"
DEFINE_TYPE(HeartBeat, ModObject);


namespace HeartBeat {

std::mutex runQueueLock;
std::queue<std::function<void(void)>> runQueue;

void InitModObject(){
    static std::once_flag initModObjFlag;
    std::call_once(initModObjFlag,[](){
        auto obj = UnityEngine::GameObject::New_ctor();
        obj->set_name("HeartBeatQuestModLifeCycleObject");
        obj->AddComponent<ModObject*>();
        DataSource::getInstance()->LateStart();
    });
}

void runInUnityThread(std::function<void ()> func){
    std::lock_guard<std::mutex> g(runQueueLock);
    runQueue.push(std::move(func));
}

void ModObject::Start(){
    UnityEngine::GameObject::DontDestroyOnLoad(gameObject);
}

void ModObject::Update(){
    {
        // run all events
        std::unique_lock<std::mutex> lock(runQueueLock);
        while(!runQueue.empty()){
            auto func = std::move(runQueue.front());
            runQueue.pop();
            lock.unlock();
            func();
            lock.lock();
        }
    }        

    HeartBeat::SettingsUI::Update();
        
    HeartBeat::DataHub::getInstance()->Update();
}


void ModObject::OnDestroy(){

}

void ModObject::OnApplicationQuit(){
    terminateBackground();
}

}