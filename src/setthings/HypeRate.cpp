#include "ModConfig.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "settings/HypeRateSettings.hpp"
#include <functional>
#include "data_sources/Hyperate.hpp"

namespace HeartBeat {

void HypeRateSettings::CreateElements() {
    auto* container = BSML::Lite::CreateScrollableSettingsContainer(controller->get_transform());

    hyperate_id = getModConfig().HypeRateId.GetValue();
    BSML::Lite::CreateText(container->get_transform(), LANG->hyperate_input_hint, 4, UnityEngine::Vector2{},
                           UnityEngine::Vector2{50, 4});

    static HMUI::InputFieldView* hyperate_id_input;
    hyperate_id_input = BSML::Lite::CreateStringSetting(container->get_transform(), "HypeRate ID", hyperate_id,
                                                        [this](StringW v) { hyperate_id = std::string(v); });

    buttons.push_back(BSML::Lite::CreateUIButton(
        container->get_transform(), LANG->hyperate_reset, UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8},
        [this]() {
            {
                hyperate_id = getModConfig().HypeRateId.GetValue();
                hyperate_id_input->set_text(hyperate_id.c_str());
            }
            hyperate_id_input->set_text(hyperate_id);

            disableBtns();
            HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatHypeRateDataSource>()->RestartSocket(
                std::bind(&HypeRateSettings::enableBtns, this));
        }));
    buttons.push_back(BSML::Lite::CreateUIButton(
        container->get_transform(), LANG->hyperate_save_and_connect, UnityEngine::Vector2{},
        UnityEngine::Vector2{50, 8}, [this]() {
            getModConfig().HypeRateId.SetValue(hyperate_id);
            disableBtns();
            HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatHypeRateDataSource>()->RestartSocket(
                std::bind(&HypeRateSettings::enableBtns, this));
        }));

    statusText = BSML::Lite::CreateText(container->get_transform(), LANG->no_message_from_server, 4,
                                        UnityEngine::Vector2{}, UnityEngine::Vector2{100, 1});
    serverMessageText = BSML::Lite::CreateText(container->get_transform(), LANG->no_message_from_server, 4,
                                               UnityEngine::Vector2{}, UnityEngine::Vector2{100, 1});
}

void HypeRateSettings::Update() {
    auto* instance = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatHypeRateDataSource>();
    statusText->set_text(instance->status.str());
    statusText->ForceMeshUpdate(false, false);
    auto t = statusText->get_rectTransform();
    t->set_sizeDelta({t->get_sizeDelta().x, statusText->get_preferredHeight()});

    if (instance->serverMessage.has_value()) {
        serverMessageText->set_text(instance->serverMessage.value());
        serverMessageText->ForceMeshUpdate(false, false);
        t = serverMessageText->get_rectTransform();
        t->set_sizeDelta({t->get_sizeDelta().x, serverMessageText->get_preferredHeight()});
    }
}

void HypeRateSettings::disableBtns() {
    for (auto& b : buttons) {
        b->set_interactable(false);
    }
}
void HypeRateSettings::enableBtns() {
    for (auto& b : buttons) {
        b->set_interactable(true);
    }
}
} // namespace HeartBeat
