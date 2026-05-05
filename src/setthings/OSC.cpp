#include "data_sources/OSC.hpp"
#include "ModConfig.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "settings/OSCSettings.hpp"
void HeartBeat::OSCSettings::CreateElements() {
    // Create a container that has a scroll bar
    auto* container = BSML::Lite::CreateVerticalLayoutGroup(controller->get_transform());

    static char osc_port[4096];
    sprintf(osc_port, LANG->heart_osc_port, getModConfig().OSCPort.GetValue());
    BSML::Lite::CreateText(container->get_transform(), osc_port, 4, UnityEngine::Vector2{},
                           UnityEngine::Vector2{50, 4});
    mDnsNameText = BSML::Lite::CreateText(container->get_transform(), LANG->mdns_name_no);

    BSML::Lite::CreateToggle(
        container->get_transform(), LANG->mdns_enable, getModConfig().OSC_MDNS_ENABLED.GetValue(), [this](bool value) {
            getModConfig().OSC_MDNS_ENABLED.SetValue(value);
            if (value) {
                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>()->StartMDns();
            } else {
                HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>()->StopMDns();
            }
        });

    osc_list = BSML::Lite::CreateScrollableList(container->get_transform(), {70, 60},
                                                [this](int idx) { UpdateSelectedOscValue(idx); });
    osc_list->set_listStyle(BSML::CustomListTableData::ListStyle::Simple);
    osc_list->tableView->set_selectionType(HMUI::TableViewSelectionType::Single);
    osc_addr.push_back("None");
    UpdateOscScrollList();
}
void HeartBeat::OSCSettings::UpdateSelectedOscValue(int idx) {
    HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>()->SetSelectedAddr(osc_addr[idx]);
    UpdateOscScrollList();
}
void HeartBeat::OSCSettings::UpdateOscScrollList() {
    auto* i = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>();
    bool any_data_changed = false;
    int the_selected = -1;
    {
        std::set<std::string> already_in(osc_addr.begin(), osc_addr.end());
        auto& devs = i->received_addresses;

        for (auto it = devs.begin(), end = devs.end(); it != end; ++it) {
            if (already_in.count(*it))
                continue;
            osc_addr.push_back(*it);
            already_in.insert(*it);
        }

        while (osc_list->data.size() > osc_addr.size()) {
            osc_list->data->RemoveAt(osc_list->data.size() - 1);
            any_data_changed = true;
        }
        while (osc_list->data.size() < osc_addr.size()) {
            osc_list->data->Add(BSML::CustomCellInfo::construct(""));
            any_data_changed = true;
        }

        for (int j = 0; j < osc_addr.size(); j++) {
            bool selected = (osc_addr[j] == i->GetSelectedAddress());
            std::string name;

            name = std::string(selected ? ">>" : "  ") + osc_addr[j];
            if (osc_list->data[j]->text != name) {
                osc_list->data[j]->text = name;
                any_data_changed = true;
            }
            if (selected) {
                the_selected = j;
            }
        }
    }
    if (any_data_changed)
        osc_list->tableView->ReloadData();
    if (the_selected >= 0) {
        osc_list->tableView->SelectCellWithIdx(the_selected, false);
    }
}

void HeartBeat::OSCSettings::Update() {
    UpdateOscScrollList();

    static std::string myMdnsName = "";
    std::string otherName = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>()->mDnsName;
    if (myMdnsName != otherName) {
        myMdnsName = otherName;
        mDnsNameText->set_text(LANG->mdns_name_title + myMdnsName);
    }
}