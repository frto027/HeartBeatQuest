#include "ModObject.hpp"
#include "UnityEngine/GameObject.hpp"
#include "DataHub.hpp"
#include "settings/Settings.hpp"
#include <mutex>
#include <queue>
#include "SettingsSnapshot.hpp"
#include "data_sources/Hyperate.hpp"

DEFINE_TYPE(HeartBeat, ModObject);


namespace HeartBeat {

std::mutex runQueueLock;
std::queue<std::function<void(void)>> runQueue;

void InitModObject(){
    static std::once_flag initModObjFlag;
    std::call_once(initModObjFlag,[](){
        UnityEngine::GameObject::New_ctor()->AddComponent<ModObject*>();
    });
}

void RunInUnityThreadLater(std::function<void ()> func){
    std::lock_guard<std::mutex> g(runQueueLock);
    runQueue.push(std::move(func));
}

void ModObject::Start(){
    UnityEngine::GameObject::DontDestroyOnLoad(gameObject);
}

void ModObject::Update(){

    {
        // run all events
        std::lock_guard<std::mutex> g(runQueueLock);
        while(!runQueue.empty()){
            runQueue.front()();
            runQueue.pop();
        }
    }

    if(MainMenuPreviewer::getInstance()->serverMessageDisplayer){
        if(SettingsSnapshot::getInstance()->DataSourceType == DS_HypeRate){
            std::string message;
            bool has_message = false;
            auto * instance = DataSource::getInstance()->as<HeartBeatHypeRateDataSource>();
            if(instance->has_message_from_server){
                std::lock_guard<std::mutex> lock(instance->message_from_server_mutex);
                if(instance->has_message_from_server){
                    message = instance->message_from_server;
                    has_message = true;
                }
            }
            if(has_message)
                MainMenuPreviewer::getInstance()->serverMessageDisplayer->set_text(message);
        }
    }
        

    HeartBeat::SettingsUI::Update();
        
    HeartBeat::DataHub::getInstance()->Update();
}


void ModObject::OnDestroy(){

}

void ModObject::OnApplicationQuit(){

}

}