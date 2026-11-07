#include "BeatLeaderRecorder.hpp"
#include "ModConfig.hpp"
#include "System/Object.hpp"
#ifdef GAME_VER_1_40_8
#include "beatsaber-hook/shared/utils/typedefs-string.hpp"
#else
#include "beatsaber-hook/shared/stringw.hpp"
#endif
#include "data_sources/DataSource.hpp"
#include "SSL10nGenerated.hpp"
#include "main.hpp"
#include "settings/MainSettings.hpp"
#include "settings/PreviewObj.hpp"
#include "settings/Settings.hpp"
#include "settings/MainSettings.hpp"
#include "SettingsSnapshot.hpp"
#include <atomic>
#include "QountersDriver.hpp"
#include "SSL10n.hpp"

std::atomic_int HeartBeat::Settings::active_setthings_ui_count = 0;

void HeartBeat::MainSettings::CreateElements() {
    HeartBeat::assetBundleMgr.Init();
    // Create a container that has a scroll bar
    auto *container = BSML::Lite::CreateScrollableSettingsContainer(controller->get_transform());

    BSML::Lite::CreateText(container->get_transform(), SSL10n::FormatKey("HEART_BEAT_QUEST_mod_version", VERSION), 4,
                           UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    BSML::Lite::CreateText(container->get_transform(), SSL10n::FormatKey("HEART_BEAT_QUEST_for_game", GAME_VERSION), 4,
                           UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8});
    std::string build_feature = "";

#ifdef WITH_REPLAY
    build_feature = build_feature + "replay ";
    // BSML::Lite::CreateText(container->get_transform(), "build-feature: replay", 4, UnityEngine::Vector2{},
    //                        UnityEngine::Vector2{50, 4});
#endif
#ifdef WITH_QOUNTERS
    build_feature = build_feature + "qounters++  ";
    // BSML::Lite::CreateText(container->get_transform(), "build-feature: qounters++", 4, UnityEngine::Vector2{},
    //                        UnityEngine::Vector2{50, 4});
    if (Qounters::Enabled()) {
        BSML::Lite::CreateText(container->get_transform(), SSL10n::Get("HEART_BEAT_QUEST_QOUNTERSPP_INIT"), 4,
                               UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    }
#endif
    if (build_feature.size()) {
        BSML::Lite::CreateText(container->get_transform(),
                               SSL10n::Get("HEART_BEAT_QUEST_setthings_build_with") + build_feature, 4,
                               UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    }

    BSML::Lite::CreateToggle(container->get_transform(), SSL10nGen::STR::enabled(), getModConfig().Enabled.GetValue(),
                             [](bool v) { getModConfig().Enabled.SetValue(v); });

    if (SettingsSnapshot::getInstance()->ModEnabled == false)
        return;

    // A data source toggle
    static std::vector<std::string_view> data_sources;
    // the value of data_sources MUST consist with the DS_*** enum
    data_sources = {"HEART_BEAT_QUEST_data_source_random",    "HEART_BEAT_QUEST_data_source_lan",
                    "HEART_BEAT_QUEST_data_source_bluetooth", "HEART_BEAT_QUEST_data_source_osc",
                    "HEART_BEAT_QUEST_data_source_hyperate",  "HEART_BEAT_QUEST_data_source_pulsoid"};

    static std::vector<std::string_view> data_sources_in_ui;
    data_sources_in_ui = {
        "HEART_BEAT_QUEST_data_source_bluetooth", "HEART_BEAT_QUEST_data_source_hyperate",
        "HEART_BEAT_QUEST_data_source_osc",       "HEART_BEAT_QUEST_data_source_pulsoid",
        "HEART_BEAT_QUEST_data_source_random",
    };

    auto current_data_type = getModConfig().DataSourceType.GetValue();
    if (current_data_type >= data_sources.size() || current_data_type == HeartBeat::DS_LAN) {
        current_data_type = HeartBeat::DS_BLE, getModConfig().DataSourceType.SetValue(current_data_type);
    }
    if (current_data_type < 0)
        current_data_type = HeartBeat::DS_BLE, getModConfig().DataSourceType.SetValue(current_data_type);
    auto dropdown =
        BSML::Lite::CreateDropdown(container->get_transform(), SSL10nGen::STR::data_source(),
                                   data_sources[current_data_type], data_sources_in_ui, [](::StringW value) {
                                       for (int i = 0; i < data_sources.size(); i++) {
                                           if (data_sources[i] == value) {
                                               getLogger().debug("{} selected", i);
                                               getModConfig().DataSourceType.SetValue(i);
                                               break;
                                           }
                                       }
                                       getLogger().debug("{} selected.", value);
                                   });
    dropdown->formatter = [](::System::Object *obj) -> StringW {
        if (!obj)
            return "null";
        return SSL10n::Get(obj->ToString());
    };
    /* apply the dropdown formatter*/
    dropdown->UpdateChoices();
    dropdown->UpdateState();

    if (HeartBeat::Recorder::BeatLeaderDetected()) {
        BSML::Lite::CreateToggle(container->get_transform(), SSL10nGen::STR::enable_record(),
                                 getModConfig().EnableRecord.GetValue(),
                                 [](bool v) { getModConfig().EnableRecord.SetValue(v); });
        BSML::Lite::CreateToggle(container->get_transform(), SSL10nGen::STR::record_dev_name(),
                                 getModConfig().RecordDevName.GetValue(),
                                 [](bool v) { getModConfig().RecordDevName.SetValue(v); });
    } else {
        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::no_beatleader(), 4, UnityEngine::Vector2{},
                               UnityEngine::Vector2{50, 8});
    }

    // the age is just used to provide a default value for maxheart
    static BSML::IncrementSetting *MaxHeartIncr;
    BSML::Lite::CreateIncrementSetting(container->get_transform(), SSL10nGen::STR::age(), 0, 1,
                                       getModConfig().Age.GetValue(), [](float v) {
                                           getModConfig().Age.SetValue(v);
                                           MaxHeartIncr->set_Value(220 - v);
                                           MaxHeartIncr->UpdateState();
                                           getModConfig().MaxHeart.SetValue(220 - v);
                                       });
    MaxHeartIncr = BSML::Lite::CreateIncrementSetting(container->get_transform(), SSL10nGen::STR::max_heart(), 0, 1,
                                                      getModConfig().MaxHeart.GetValue(),
                                                      [](float v) { getModConfig().MaxHeart.SetValue(v); });

    std::vector<std::string_view> ui_s;
    for (auto &pair : HeartBeat::assetBundleMgr.loadedBundles) {
        ui_s.push_back(pair.first);
    }

    static HMUI::CurvedTextMeshPro *feature_unsupport_hint_ui;

    BSML::Lite::CreateDropdown(
        container->get_transform(), SSL10nGen::STR::select_ui(), getModConfig().SelectedUI.GetValue(), ui_s,
        [](StringW v) {
            if (getModConfig().SelectedUI.GetValue() != v) {
                getModConfig().SelectedUI.SetValue(v);

                MainMenuPreviewer::getInstance()->Reload();

                bool supported = true;
                bool need_advanced_ui = false;
                {
                    HeartBeat::assetBundleMgr.Init();
                    auto it = HeartBeat::assetBundleMgr.loadedBundles.find(v);
                    if (it != HeartBeat::assetBundleMgr.loadedBundles.end()) {
                        auto &features = it->second.unsupported_features;
                        if (features.size() > 0)
                            supported = false;
                        auto &infos = it->second.infos;
                        if (infos.contains("root")) {
                            auto &root = infos["root"];
                            if (root == "songProgressPanelGO" || root == "relativeScoreGo" ||
                                root == "immediateRankGo") {
                                need_advanced_ui = true;
                            }
                        }
                    }
                }
                if (supported) {
                    if (need_advanced_ui) {
                        feature_unsupport_hint_ui->set_text(SSL10nGen::STR::advance_ui_required());
                    } else {
                        feature_unsupport_hint_ui->set_text("");
                    }
                } else {
                    feature_unsupport_hint_ui->set_text(SSL10nGen::STR::unsupported_feature_udpatre_mod());
                }
            }
        });

    feature_unsupport_hint_ui =
        BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    feature_unsupport_hint_ui->set_color(UnityEngine::Color::get_red());

    private_public_btn = BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::waiting(),
                                                    UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8}, [this]() {
                                                        private_ui = !private_ui;
                                                        UpdateContent();
                                                    });

    BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::your_setthings_is_in_another_menu(), 4,
                           UnityEngine::Vector2{}, UnityEngine::Vector2{50, 10});

    UpdateContent();
}

void HeartBeat::MainSettings::UpdateContent() {
    BSML::Lite::SetButtonText(private_public_btn,
                              private_ui ? SSL10nGen::STR::hiding_mac_addr() : SSL10nGen::STR::showing_mac_addr());
}
