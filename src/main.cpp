#include "main.hpp"
#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "HeartBeat.hpp"
#include "ModObject.hpp"
#include "SettingsSnapshot.hpp"
#include "data_sources/Bluetooth.hpp"
#include "data_sources/DataSource.hpp"
#include "settings/Settings.hpp"
#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "QountersDriver.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/zzzz__GameObject_def.hpp"
#include "bsml/shared/BSML.hpp"
#include "multi_version_compat.hpp"

#include "HMUI/HierarchyManager.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"

#include "UnityEngine/Quaternion.hpp"

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/register.hpp"

#include "QountersDriver.hpp"

#include "ModConfig.hpp"

#include "i18n.hpp"
#include <cstddef>
#include <mutex>
#include "BeatLeaderRecorder.hpp"

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0}; // Stores the ID and version of our mod, and is sent to the modloader upon startup
std::string modConfigFilePath = "unk";

// Called at the early stages of game loading
extern "C" void setup(CModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo.assign(info);
	
    getModConfig().Init(modInfo);

    modConfigFilePath = Configuration::getConfigFilePath(modInfo);

    getLogger().info("Completed setup!");

    HeartBeat::SettingsSnapshot::getInstance();

    if(HeartBeat::SettingsSnapshot::getInstance()->ModEnabled){
        HeartBeat::DataSource::getInstance();
    }
}

Paper::ConstLoggerContext<21> & getLogger(){
    static Paper::ConstLoggerContext<21> logger = Paper::ConstLoggerContext("HeartBeatLanReceiver");
    return logger;
}
MAKE_HOOK_MATCH(GameplayCoreHook, &GlobalNamespace::CoreGameHUDController::Initialize, void, GlobalNamespace::CoreGameHUDController * self, GlobalNamespace::CoreGameHUDController::InitData * data){
    GameplayCoreHook(self, data);

    static int firstInitialize = true;
    if(firstInitialize){
        firstInitialize = false;
        if(HeartBeat::SettingsSnapshot::getInstance()->DataSourceType == HeartBeat::DS_BLE){
            HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatBleDataSource>()->SetSelectedBleMac(getModConfig().SelectedBleMac.GetValue());
        }
    }

    #ifdef WITH_QOUNTERS
    if(HeartBeat::Qounters::Enabled() && !getModConfig().IgnoreQounters.GetValue()){
        getLogger().info("Qounters enabled, will not load mod UI.Loading qounters feeder object");
        HeartBeat::AssetBundleInstinateInformation result; // there is no asset bundle with qounters
        result.gameObject = UnityEngine::GameObject::New_ctor();
        auto comp = result.gameObject->AddComponent<HeartBeat::HeartBeatObj*>();
        comp->loadedComponents = result;
        comp->isQountersMode = true;
        return;
    }
    #endif

    HeartBeat::assetBundleMgr.Init();

    std::string SelectedUI = getModConfig().SelectedUI.GetValue();
    if(!HeartBeat::assetBundleMgr.loadedBundles.contains(SelectedUI))
        SelectedUI = "Default";
    if(!HeartBeat::assetBundleMgr.loadedBundles.contains(SelectedUI)){
        getLogger().error("Can't find ui asset bundle '{}' to load!", SelectedUI);
        return;
    }

    getLogger().info("Loading '{}' at game start", SelectedUI);

    UnityEngine::GameObject * parent = self->get_energyPanelGo();
    auto & assetUI = HeartBeat::assetBundleMgr.loadedBundles[SelectedUI];
    if(assetUI.infos.contains("root")){
        std::string root_str = assetUI.infos["root"];
        if(root_str == "energyPanelGo") parent = self->get_energyPanelGo();
        else if(root_str == "songProgressPanelGO") parent = self->get_songProgressPanelGO();
        else if(root_str == "relativeScoreGo") parent = self->get_relativeScoreGo();
        else if(root_str == "immediateRankGo") parent = self->get_immediateRankGo();
        else getLogger().info("unknown position {}, attach it to energyPanelGo", root_str);
    }
    getLogger().info("UI Mount position: {}", parent->get_name());

    HeartBeat::AssetBundleInstinateInformation result;
    if(!HeartBeat::assetBundleMgr.Instantiate(SelectedUI, parent->get_transform(), result)){
        getLogger().error("The UI Can't loaded.");
        return;
    }
    auto comp = result.gameObject->AddComponent<HeartBeat::HeartBeatObj*>();
    comp->loadedComponents = result;
    comp->isQountersMode = false;
    getLogger().info("The UI has been created");
}

MAKE_HOOK_MATCH(HeartBeatSceneChange, &UnityEngine::SceneManagement::SceneManager::SetActiveScene, bool, UnityEngine::SceneManagement::Scene scene){
    
    // maybe I could find a better hook site in the future.
    static std::once_flag in_game_init_flag;
    std::call_once(in_game_init_flag, [](){
        HeartBeat::InitModObject();
    });
    
    return HeartBeatSceneChange(scene);
}


// Called later on in the game loading - a good time to install function hooks
extern "C" void late_load() {
    getLogger().info("Loading HeartBeatQuest(" VERSION ","  GAME_VERSION ")");

    il2cpp_functions::Init();

    custom_types::Register::AutoRegister();

    getLogger().info("init BSML");
    BSML::Init();

    getLogger().info("init i18n");
    I18N::Setup();

    getLogger().info("Installing ui...");
    HeartBeat::SettingsUI::Setup();

    if(HeartBeat::SettingsSnapshot::getInstance()->ModEnabled == false){
        getLogger().info("The mod is not enabled");
        return;
    }


    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), GameplayCoreHook);
    INSTALL_HOOK(getLogger(), HeartBeatSceneChange);

    getLogger().info("init recorder...");
    HeartBeat::Recorder::Init();

    #ifdef WITH_QOUNTERS
    getLogger().info("try init qounters...");
    HeartBeat::Qounters::Init();
    #endif
    
    getLogger().info("Done.");
}