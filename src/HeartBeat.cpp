#include "ModConfig.hpp"
#include "QountersDriver.hpp"
#include "SettingsSnapshot.hpp"
#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "TMPro/TextMeshPro.hpp"
#include "UnityEngine/Animator.hpp"
#include "UnityEngine/Color.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "data_sources/Hyperate.hpp"
#include "main.hpp"
#include "HeartBeat.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Quaternion.hpp"

#include "multi_version_compat.hpp"
#include "DataHub.hpp"

#include "BeatLeaderRecorder.hpp"

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Resources.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "data_sources/DataSource.hpp"
#include "settings/PreviewObj.hpp"
#include "settings/Settings.hpp"
#include "stdio.h"
#include <cstddef>
#include <mutex>
#include "UIManager.hpp"

DEFINE_TYPE(HeartBeat, HeartBeatObj);

const char *HeartBeat::ui_features[] = {
    "digit_anim",
    "datasource_anim",
    NULL
};

namespace HeartBeat{
    

    void HeartBeatObj::Start(){
        addToUIManager();
    }

    void HeartBeatObj::addToUIManager(){
        if(isAddedToUIManager)
            return;
        isAddedToUIManager = true;
        UIManager::getInstance()->addReader();
    }
    void HeartBeatObj::removeFromUIManager(){
        if(!isAddedToUIManager)
            return;
        isAddedToUIManager = false;
        UIManager::getInstance()->decReader();
    }

    void HeartBeatObj::OnDestroy(){
        removeFromUIManager();

        // we will disable replay here (when the UI inside the game scene is destroyed), because I don't want hook a scene unload function.
        HeartBeat::Recorder::replayStarted = false;

    }
    void HeartBeatObj::Update(){

        #if WITH_QOUNTERS
        if(isQountersMode){
            DataHub::getInstance()->Update();
            int data;
            if(DataHub::getInstance()->GetData(data))
                HeartBeat::Qounters::DisplayData(data);
            //there is no asset bundle UI for qounters, just return.
            return;
        }
        #endif

        if(this->gameObject->activeInHierarchy == false)
            return;

        HeartBeat::DataHub::getInstance()->Update();
        int data;
        if(DataHub::getInstance()->GetData(data)){
            int Maximum = getModConfig().MaxHeart.GetValue();
            float percent = ((float)data) / Maximum;


            char buff[256];
            sprintf(buff, "%d", data);
            for(auto text : loadedComponents.heartrateTexts)
                text->set_text(buff);
            for(auto anmt : loadedComponents.animators){
                //loadedComponents.animator->SetInteger("age", 25);
                anmt->SetInteger("heartrate", data);
                anmt->SetFloat("heartpercent", percent);
                anmt->SetTrigger("datacome");
                anmt->SetBool("replaying", HeartBeat::Recorder::isReplaying());
                
                anmt->SetInteger("hr_1", data % 10);
                anmt->SetInteger("hr_10", (data/10)%10);
                anmt->SetInteger("hr_100", (data/100)%10);

                anmt->SetInteger("datasource", HeartBeat::DataSource::getInstance()->dataSourceType);
            }
        }
    }
};

#define ASSET_UI_PATH "/sdcard/ModData/com.beatgames.beatsaber/Mods/HeartBeatQuest/UI/"
#define DEFAULT_UI_PATH "/sdcard/ModData/com.beatgames.beatsaber/Mods/HeartBeatQuest/defaultUI.bundle"
namespace HeartBeat{
    AssetBundleManager assetBundleMgr;
    
    void FixPrefab(UnityEngine::Transform * transform){
        auto tm = transform->GetComponent<TMPro::TMP_Text *>();
        if(tm){
            tm->set_font(BSML::Helpers::GetMainTextFont());
            tm->set_fontSharedMaterial(BSML::Helpers::GetMainUIFontMaterial());
        }
        for(int i=0;i<transform->get_childCount();i++){
            FixPrefab(transform->GetChild(i).ptr());
        }

    }

    void AssetBundleManager::Init(){
        if(initialized)return;
        initialized = true;

        auto LoadAssetBundle = [this](UnityEngine::AssetBundle* bundle, std::optional<std::string> filepath){
            auto assetNamesUnity = bundle->GetAllAssetNames();
            std::vector<std::string> assetPaths = {assetNamesUnity->begin(), assetNamesUnity->end()};
            for(auto assetPath : assetPaths){
                getLogger().info("Start load {}", assetPath);
                #if defined(GAME_VER_1_28_0) || defined(GAME_VER_1_35_0)
                auto // SafePtrUnity not work here, just use auto for compat
                #else
                SafePtrUnity<UnityEngine::GameObject> 
                #endif
                    gameObject = bundle->LoadAsset<UnityEngine::GameObject*>(assetPath);
                if(gameObject){
                    auto info = gameObject->get_transform()->Find("info");
                    if(!info)
                        continue;
                    
                    std::map<std::string, std::string> infos = {};

                    for(int i=0;i<info->get_childCount();i++){
                        auto name = info->GetChild(i)->get_name();
                        if(name){
                            auto col = name->IndexOf(':');
                            if(col > 0){
                                auto key = name->Substring(0, col);
                                auto val = name->Substring(col+1);
                                std::string key_str = key;
                                auto old_val_it = infos.find(key_str);
                                if(old_val_it == infos.end())
                                    infos[key_str] = std::string(val);
                                else
                                    infos[key_str] = old_val_it->second + "," + std::string(val);
                            }
                        }
                    }
                    
                    std::string name = "NoName";
                    if(infos.contains("name"))
                        name = infos["name"];
                    if(loadedBundles.contains(name)){
                        size_t malloc_size = name.size() + 10;
                        char * buff = (char*)malloc(malloc_size);
                        for(int i=2;i<100;i++){
                            sprintf(buff, "%s %d", name.c_str(), i);
                            if(!loadedBundles.contains(buff))
                                break;
                        }
                        name = buff;
                        free(buff);
                    }
                    if(loadedBundles.contains(name)){
                        continue;
                    }

                    std::set<std::string> unsupported_features;
                    std::set<std::string> supported_features;
                    if(infos.contains("feature")){
                        unsupported_features = GetFeatures(infos["feature"]);
                    }
                    
                    for(const char ** feature = ui_features; *feature; feature++){
                        auto it = unsupported_features.find(*feature);
                        if(it != unsupported_features.end()){
                            supported_features.insert(*feature);
                            unsupported_features.erase(it);
                        }
                    }
                    getLogger().info("Loaded UI, asset name: '{}'", name);
                    if(unsupported_features.size() > 0){
                        getLogger().info("  {} features are unsupported.", unsupported_features.size());
                        for(auto & feature : unsupported_features){
                            getLogger().info("    feature unsupported: {}", feature);
                        }
                    }
                    loadedBundles.insert({name, {filepath, assetPath, std::move(infos), std::move(supported_features), std::move(unsupported_features)}});
                }
            }
            getLogger().info("bundle load over");
        };
        
        try{
            auto bundle = UnityEngine::AssetBundle::LoadFromFile(DEFAULT_UI_PATH);
            LoadAssetBundle(bundle, {});
            getLogger().info("Unload bundle {}", (void*)bundle);
            bundle->Unload(true);
            getLogger().info("done");
        }catch(...){
            getLogger().error("Can't load default ui");
        }
        getLogger().info("Start loading bundles from directory");
        if(std::filesystem::is_directory(ASSET_UI_PATH)){

            for(auto& entry :std::filesystem::directory_iterator(ASSET_UI_PATH)){
                getLogger().info("Handling {}", entry.path().c_str());
                if(entry.is_regular_file() && entry.path().has_extension() && entry.path().extension() == ".bundle"){
                    try{
                        auto bundle = UnityEngine::AssetBundle::LoadFromFile(entry.path().c_str());
                        LoadAssetBundle(bundle, entry.path());
                        bundle->Unload(true);
                    }catch(...){
                        getLogger().error("Can't load asset file {}", entry.path().c_str());
                    }
                }
            }
        }
        getLogger().info("directory load over");
    }

    void HandleTransformsInBundle(AssetBundleInstinateInformation & result, UnityEngine::Transform * transform){
        {
            auto tm = transform->GetComponent<TMPro::TMP_Text *>();
            if(tm){
                if(transform->get_name()->Equals("auto:heartrate")){
                    result.heartrateTexts.push_back(tm);
                }
            }
            auto anmt = transform->GetComponent<UnityEngine::Animator*>();
            if(anmt){
                result.animators.push_back(anmt);
            }
        }
        for(int i=0;i<transform->get_childCount();i++){
            HandleTransformsInBundle(result, transform->GetChild(i).ptr());
        }
    }

    bool AssetBundleManager::Instantiate(std::string name, UnityEngine::Transform * parent, AssetBundleInstinateInformation & result){
        if(!loadedBundles.contains(name))
            return false;
        auto & assetUI = loadedBundles[name];

        UnityEngine::AssetBundle * bundle = nullptr;
        {
            if(assetUI.filePath.has_value()){
                try{
                    bundle = UnityEngine::AssetBundle::LoadFromFile(assetUI.filePath.value());

                }catch(...){
                    getLogger().error("Can't load asset bundle {}", assetUI.filePath.value());
                }
            }else{
                try{
                    bundle = UnityEngine::AssetBundle::LoadFromFile(DEFAULT_UI_PATH);
                }catch(...){
                    getLogger().error("Can't load default ui");
                }
            }
        }

        if(bundle == nullptr){
            getLogger().error("UI AssetBundle load failed");
            return false;
        }

        UnityEngine::GameObject * prefab = bundle->LoadAsset<UnityEngine::GameObject *>(assetUI.AssetPath);
        if(prefab == nullptr){
            getLogger().error("Can't load prefab {}", assetUI.AssetPath);
            bundle->Unload(true);
            return false;
        }

        FixPrefab(prefab->get_transform());
        auto gameobject = UnityEngine::GameObject::Instantiate(prefab, parent);
        getLogger().info("InstinateDone");
        bundle->Unload(false);
        HandleTransformsInBundle(result, gameobject->get_transform());
        result.gameObject = gameobject;
        return true;
    }

    std::set<std::string> AssetBundleManager::GetFeatures(std::string feature){
        std::set<std::string> features;

        size_t before = 0;
        while(before < feature.size()){
            size_t next = feature.find(',', before);
            if(next == before){
                before = next + 1;
                continue;
            }
            if(next == std::string::npos){
                features.insert(feature.substr(before));
                break;
            }else{
                features.insert(feature.substr(before, next - before));
                before = next + 1;
            }
        }
        return std::move(features);
    }
}