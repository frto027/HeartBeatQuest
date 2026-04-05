#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include <atomic>
#include <il2cpp-config.h>
#include "HeartBeat.hpp"
#include <string>
#include <vector>
#include "bsml/shared/BSML.hpp"
#include "PreviewObj.hpp"

namespace HeartBeat{

    extern bool private_ui;

    void OpenWebpage(std::string url);

    class Settings{
    public:
        static std::atomic_int active_setthings_ui_count;
    private:
        std::string menuTitle, buttonText,hoverHint;

        bool m_isActive = false;

    protected:
        HMUI::ViewController *controller = nullptr;

    public:
        Settings(const std::string menuTitle, const std::string buttonText, const std::string hoverHint)
        :menuTitle(menuTitle),buttonText(buttonText),hoverHint(hoverHint)
        {
        }

        void Register(){
            BSML::Register::RegisterMainMenuViewControllerMethod(menuTitle, buttonText, hoverHint, [this](HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
                if(firstActivation){
                    this->controller = self;
                    this->CreateElements();
                    self->add_didDeactivateEvent(custom_types::MakeDelegate<HMUI::ViewController::DidDeactivateDelegate*>(std::function([this](bool removedFromHierarchy, bool screenSystemDisabling){
                        MainMenuPreviewer::getInstance()->Hide();
                        m_isActive = false;
                        active_setthings_ui_count--;
                        this->Close();
                    })));
                }
                this->Open();
                m_isActive = true;
                active_setthings_ui_count++;
                MainMenuPreviewer::getInstance()->Show();
            });
        }

        bool isActive(){return m_isActive;}
        virtual void CreateElements() = 0;
        virtual void Open() {};
        virtual void Close() {};

        // called repeatedly when enabled by the UI objects from SettingsUI::Update
        virtual void Update() {};
    };

    namespace SettingsUI{
        void Setup();
        // called from active game objects
        void Update();

        extern std::vector<Settings *> settings;
    }
}
