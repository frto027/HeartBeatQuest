#include "ModConfig.hpp"
#include "ModObject.hpp"
#include "main.hpp"
#include "data_sources/Pulsoid.hpp"
#include "settings/PulsoidSettings.hpp"
#include <string>

void HeartBeat::PulsoidSettings::CreateElements() {
    // self->add_didDeactivateEvent(custom_types::MakeDelegate<HMUI::ViewController::DidDeactivateDelegate*>(std::function([](bool
    // removedFromHierarchy, bool screenSystemDisabling){
    //     if(MainMenuPreviewObject) MainMenuPreviewObject->set_active(false);
    //     MainMenuPreviewObjectComp->removeFromUIManager();
    // })));
    // devices_controller = self;
    // Create a container that has a scroll bar
    auto* container = BSML::Lite::CreateScrollableSettingsContainer(controller->get_transform());

    BSML::Lite::CreateUIButton(
        container->get_transform(), LANG->pulsoid_reconnect, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8},
        []() { HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->ResetConnection(); });

    statusText =
        BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 1});

    BSML::Lite::CreateText(container->get_transform(), LANG->pulsoid_input_hint, 4, UnityEngine::Vector2{},
                           UnityEngine::Vector2{50, 4});

    {
        auto* pair_container = BSML::Lite::CreateHorizontalLayoutGroup(container->get_transform());
        PairInBrowserBtn = BSML::Lite::CreateUIButton(
            pair_container->get_transform(), LANG->pulsoid_connect, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8},
            [this]() {
                setButtonOpeningUrl();
                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->RequestSafePair(
                    [this]() {
                        // ondone
                        setButtonPairing();
                    },
                    [this](std::string errmsg) {
                        // onfail
                        this->errMsgText->set_text(errmsg);
                        setButtonPairDone();
                    });
            });

        BrowserCompleteBtn = BSML::Lite::CreateUIButton(
            pair_container->get_transform(), LANG->pulsoid_done, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8},
            [this]() {
                setButtonBusying();
                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->SafePairDone(
                    [this]() {
                        runInUnityThread([this]() {
                            setButtonPairDone();
                            this->errMsgText->set_text("");
                            this->SyncModConfig();
                            HeartBeat::DataSource::getInstance()
                                ->as<HeartBeat::HeartBeatPulsoidDataSource>()
                                ->ResetConnection();
                        });
                    },
                    [this]() {
                        // pending, donothing
                        runInUnityThread([this]() {
                            setButtonPairing();
                            this->errMsgText->set_text("");
                        });
                    },
                    [this](std::string failmsg) {
                        runInUnityThread([this, failmsg]() {
                            this->errMsgText->set_text(failmsg);
                            setButtonPairDone();
                        });
                    });
            });

        CancelBrowserPairBtn = BSML::Lite::CreateUIButton(
            pair_container->get_transform(), LANG->pulsoid_cancel, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8},
            [this]() {
                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->SafePairCancel();
                setButtonPairDone();
            });
        BrowserCompleteBtn->set_interactable(false);
        CancelBrowserPairBtn->set_interactable(false);
    }

    errMsgText =
        BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    errMsgText->set_color(UnityEngine::Color::get_red());

    BSML::Lite::CreateText(container->get_transform(), LANG->pulsoid_token, 4, UnityEngine::Vector2{},
                           UnityEngine::Vector2{50, 4});
    tokenText =
        BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
    BSML::Lite::CreateUIButton(
        container->get_transform(), LANG->pulsoid_clear_token, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8},
        [this]() {
            getModConfig().PulsoidToken.SetValue(getModConfig().PulsoidToken.GetDefaultValue());
            this->SyncModConfig();
            HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->ResetConnection();
        });

    {
        std::string config_file_hint = LANG->pulsoid_edit_config_hint + modConfigFilePath;
        char buff[1024];
        int j = 0;
        for (int i = 0, ch_count = 0; i < config_file_hint.size(); i++) {
            // please we are handling utf8 string
            if ((config_file_hint[i] & 0xC0) != 0x80) {
                ch_count++;
                auto ch = config_file_hint[i];
                if (ch_count > 40 && (ch == ' ' || ch == '/' || ch == '\\')) {
                    buff[j++] = '\n';
                    ch_count = 0;
                }
                if ((config_file_hint[i] & 0x80))
                    ch_count += 1;

                if (config_file_hint[i] == '\n')
                    ch_count = 0;
            }
            buff[j++] = config_file_hint[i];
        }
        buff[j++] = '\0';
        BSML::Lite::CreateText(container->get_transform(), buff, 4, UnityEngine::Vector2{},
                               UnityEngine::Vector2{50, 18});
    }

    SyncModConfig();
}

void HeartBeat::PulsoidSettings::SyncModConfig() {
    std::string token = getModConfig().PulsoidToken.GetValue();
    for (int i = 8; i < token.size(); i++) {
        if (token[i] != '-')
            token[i] = '*';
    }
    tokenText->set_text(token);
}
void HeartBeat::PulsoidSettings::setButtonBusying() {
    PairInBrowserBtn->set_interactable(false);
    BrowserCompleteBtn->set_interactable(false);
    CancelBrowserPairBtn->set_interactable(false);
}
void HeartBeat::PulsoidSettings::setButtonOpeningUrl() {
    PairInBrowserBtn->set_interactable(false);
    BrowserCompleteBtn->set_interactable(false);
    CancelBrowserPairBtn->set_interactable(false);
}
void HeartBeat::PulsoidSettings::setButtonPairing() {
    PairInBrowserBtn->set_interactable(false);
    BrowserCompleteBtn->set_interactable(true);
    CancelBrowserPairBtn->set_interactable(true);
}
void HeartBeat::PulsoidSettings::setButtonPairDone() {
    PairInBrowserBtn->set_interactable(true);
    BrowserCompleteBtn->set_interactable(false);
    CancelBrowserPairBtn->set_interactable(false);
}

void HeartBeat::PulsoidSettings::Update() {
    auto str = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->status.str();
    if (statusText->get_text() != str) {
        statusText->set_text(str);
        statusText->ForceMeshUpdate(false, false);
        auto r = statusText->get_rectTransform();
        r->set_sizeDelta({r->get_sizeDelta().x, statusText->get_preferredHeight()});
    }
}

void HeartBeat::PulsoidSettings::Close() {
    setButtonPairDone();
    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->SafePairCancel();
}