#include "settings/PreviewObj.hpp"
#include "HeartBeat.hpp"
#include "ModConfig.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "UnityEngine/Vector3.hpp"
#include "main.hpp"
#include "settings/PreviewObj.hpp"

HeartBeat::MainMenuPreviewer::MainMenuPreviewer(){
    Reload();
    Hide();
}

void HeartBeat::MainMenuPreviewer::Show(){
    MainMenuPreviewObject->set_active(true);
    MainMenuPreviewObjectComp->addToUIManager();
}

void HeartBeat::MainMenuPreviewer::Hide(){
    MainMenuPreviewObjectComp->removeFromUIManager();
    MainMenuPreviewObject->set_active(false);
}

void HeartBeat::MainMenuPreviewer::Reload(){
    if(MainMenuPreviewObject){
        UnityEngine::Object::Destroy(MainMenuPreviewObject);
        MainMenuPreviewObject = nullptr;
        MainMenuPreviewObjectComp = nullptr;
    }

    HeartBeat::assetBundleMgr.Init();
    auto obj = UnityEngine::GameObject::New_ctor();
    UnityEngine::Object::DontDestroyOnLoad(obj);
    auto canvas = obj->AddComponent<UnityEngine::Canvas*>();
    canvas->set_renderMode(UnityEngine::RenderMode::WorldSpace);
    canvas->set_scaleFactor(0.001);
    
    obj->AddComponent<HMUI::CurvedCanvasSettings*>()->_radius = 0;

    auto crect = canvas->GetComponent<UnityEngine::RectTransform*>();
    crect->set_position({1, 1.5, 3});
    crect->set_localScale({0.01f,0.01f,0.01f});

    std::string SelectedUI = getModConfig().SelectedUI.GetValue();
    if(!HeartBeat::assetBundleMgr.loadedBundles.contains(SelectedUI))
        SelectedUI = "Default";
    if(!HeartBeat::assetBundleMgr.loadedBundles.contains(SelectedUI)){
        getLogger().error("Can't find ui asset bundle '{}' to load!", SelectedUI);
    }

    HeartBeat::AssetBundleInstinateInformation result;
    HeartBeat::assetBundleMgr.Instantiate(SelectedUI, canvas->get_transform(), result);
    MainMenuPreviewObjectComp = result.gameObject->AddComponent<HeartBeat::HeartBeatObj*>();
    MainMenuPreviewObjectComp->loadedComponents = result;
    MainMenuPreviewObjectComp->isQountersMode = false;

    MainMenuPreviewObject = obj;
}

HeartBeat::MainMenuPreviewer * HeartBeat::MainMenuPreviewer::getInstance(){
    static HeartBeat::MainMenuPreviewer * instance = nullptr;
    if(!instance)
        instance = new HeartBeat::MainMenuPreviewer();
    return instance;
}
