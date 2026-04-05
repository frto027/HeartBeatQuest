#include "ModConfig.hpp"
#include "main.hpp"
#include "data_sources/Pulsoid.hpp"
#include "settings/PulsoidSettings.hpp"
#include "settings/Settings.hpp"

void HeartBeat::PulsoidSettings::CreateElements(){
                // self->add_didDeactivateEvent(custom_types::MakeDelegate<HMUI::ViewController::DidDeactivateDelegate*>(std::function([](bool removedFromHierarchy, bool screenSystemDisabling){
                //     if(MainMenuPreviewObject) MainMenuPreviewObject->set_active(false);
                //     MainMenuPreviewObjectComp->removeFromUIManager();
                // })));
                // devices_controller = self;
                // Create a container that has a scroll bar
                auto *container = BSML::Lite::CreateVerticalLayoutGroup(controller->get_transform());

                BSML::Lite::CreateUIButton(container->get_transform(), LANG->pulsoid_reconnect, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8}, [](){
                    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->ResetConnection();
                });

                BSML::Lite::CreateText(container->get_transform(), LANG->pulsoid_input_hint, 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
                
                {
                    auto * pair_container = BSML::Lite::CreateHorizontalLayoutGroup(container->get_transform());
                    PairInBrowserBtn = BSML::Lite::CreateUIButton(pair_container->get_transform(), LANG->pulsoid_connect, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8}, [](){
                        HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->RequestSafePair();
                    });

                    BrowserCompleteBtn = BSML::Lite::CreateUIButton(pair_container->get_transform(), LANG->pulsoid_done, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8}, [](){
                        HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->SafePairDone();
                    });

                    CancelBrowserPairBtn = BSML::Lite::CreateUIButton(pair_container->get_transform(), LANG->pulsoid_cancel, UnityEngine::Vector2{}, UnityEngine::Vector2{20, 8}, [](){
                        HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->CancelSafePair();
                    });
                    BrowserCompleteBtn->set_interactable(false);
                    CancelBrowserPairBtn->set_interactable(false);
                }

                errMsgText = BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
                errMsgText->set_color(UnityEngine::Color::get_red());

                BSML::Lite::CreateText(container->get_transform(), LANG->pulsoid_token, 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
                tokenText = BSML::Lite::CreateText(container->get_transform(), "", 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 4});
                BSML::Lite::CreateUIButton(container->get_transform(), LANG->pulsoid_clear_token, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8}, [](){
                    getModConfig().PulsoidToken.SetValue(getModConfig().PulsoidToken.GetDefaultValue());
                    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->modconfig_is_dirty = true;
                    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->ResetConnection();
                });

                {
                    std::string config_file_hint = LANG->pulsoid_edit_config_hint + modConfigFilePath;
                    char buff[1024];
                    int j = 0;
                    for(int i=0, ch_count = 0;i<config_file_hint.size();i++){
                        //please we are handling utf8 string
                        if((config_file_hint[i] & 0xC0) != 0x80){
                            ch_count++;
                            auto ch = config_file_hint[i];
                            if(ch_count > 40 && (ch == ' ' || ch == '/' || ch == '\\')){
                                buff[j++] = '\n';
                                ch_count = 0;
                            }
                            if((config_file_hint[i] & 0x80))
                                ch_count += 1;

                            if(config_file_hint[i] == '\n')
                                ch_count = 0;
                        }
                        buff[j++] = config_file_hint[i];
                    }
                    buff[j++] = '\0';
                    BSML::Lite::CreateText(container->get_transform(), buff, 4, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 18});
                }

                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->modconfig_is_dirty = true;
}

void HeartBeat::PulsoidSettings::Update(){
    auto * ds = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>();
    if(ds->modconfig_is_dirty){
        ds->modconfig_is_dirty = false;
        
        {
            std::string token = getModConfig().PulsoidToken.GetValue();
            for(int i=8;i<token.size();i++){
                if(token[i] != '-')
                    token[i] = '*';
            }
            tokenText->set_text(token);
        }
    }

    if(ds->err_message_dirty){
        std::lock_guard<std::mutex> g(ds->err_message_mutex);
        errMsgText->set_text(ds->err_message);
        ds->err_message_dirty = false;
    }
    if(ds->url_open_wanted){
        std::string url;
        {
            std::lock_guard<std::mutex> g(ds->url_mutex);
            url = ds->url;
            ds->url_open_wanted = false;
        }
        //open the login url in the quest browser
        OpenWebpage(url);
    }
    if(PairInBrowserBtn){
        PairInBrowserBtn->set_interactable(!ds->IsSafePairing());
        BrowserCompleteBtn->set_interactable(ds->IsSafePairing());
        CancelBrowserPairBtn->set_interactable(ds->IsSafePairing());
    }
}

void HeartBeat::PulsoidSettings::Close(){
    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatPulsoidDataSource>()->CancelSafePair();
}