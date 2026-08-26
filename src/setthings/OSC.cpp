#include "data_sources/OSC.hpp"
#include "ModConfig.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "SSL10nGenerated.hpp"
#include "settings/OSCSettings.hpp"
void HeartBeat::OSCSettings::CreateElements() {
    // Create a container that has a scroll bar
    auto *container = BSML::Lite::CreateVerticalLayoutGroup(controller->get_transform());
    oscDataSource = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>();

    BSML::Lite::CreateText(container->get_transform(),
                           SSL10nGen::FMT::heart_osc_port(getModConfig().OSCPort.GetValue()), 4, UnityEngine::Vector2{},
                           UnityEngine::Vector2{50, 4});
    mDnsNameText = BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::mdns_name_no());

    BSML::Lite::CreateToggle(container->get_transform(), SSL10nGen::STR::mdns_enable(),
                             getModConfig().OSC_MDNS_ENABLED.GetValue(), [this](bool value) {
                                 getModConfig().OSC_MDNS_ENABLED.SetValue(value);
                                 if (value) {
                                     oscDataSource->StartMDns();
                                 } else {
                                     oscDataSource->StopMDns();
                                 }
                             });

    osc_list = BSML::Lite::CreateScrollableList(container->get_transform(), {70, 60},
                                                [this](int idx) { UpdateSelectedOscValue(idx); });
    osc_list->set_listStyle(BSML::CustomListTableData::ListStyle::Simple);
    osc_list->tableView->set_selectionType(HMUI::TableViewSelectionType::Single);
    UpdateOscScrollList();
}

DEFINE_TYPE(HeartBeat, OSCDeviceItem);
HeartBeat::OSCDeviceItem *HeartBeat::OSCDeviceItem::construct() {
    auto ret = OSCDeviceItem::New_ctor();
    ret->text = "";
    ret->subText = nullptr;
    ret->icon = nullptr;
    return ret;
}

void HeartBeat::OSCSettings::UpdateSelectedOscValue(int idx) {
    oscDataSource->SetSelectedAddr(((OSCDeviceItem *)osc_list->data->get_Item(idx))->devAddress);
    UpdateOscScrollList();
}
void HeartBeat::OSCSettings::UpdateOscScrollList() {
    bool changed = false;
    std::string selected = oscDataSource->GetSelectedAddress();
    if (osc_list->data.size() == 0) {
        auto dev = HeartBeat::OSCDeviceItem::construct();
        dev->isNone = true;
        osc_list->data.push_back(dev);
        changed = true;
    }

    int select_cell_idx = -1;
    int index = 1;

    OSCDeviceItem *noneItem = (OSCDeviceItem *)osc_list->data->get_Item(0);
    if (selected == "")
        select_cell_idx = 0;
    changed |= noneItem->Update("", selected == "");

    for (auto address : oscDataSource->received_addresses) {
        OSCDeviceItem *listItem;
        if (osc_list->data.size() <= index) {
            listItem = OSCDeviceItem::construct();
            osc_list->data.push_back(listItem);
            changed = true;
        } else {
            listItem = (OSCDeviceItem *)osc_list->data->get_Item(index);
        }

        bool isSelected = selected == address;
        changed |= listItem->Update(address, isSelected);
        if (isSelected)
            select_cell_idx = index;
        index++;
    }
    if (index < osc_list->data.size()) {
        osc_list->data->RemoveRange(index, osc_list->data.size() - index);
        changed = true;
    }
    if (changed) {
        osc_list->tableView->ReloadData();
    }

    if (select_cell_idx >= 0) {
        osc_list->tableView->SelectCellWithIdx(select_cell_idx, false);
    }
}

void HeartBeat::OSCSettings::Update() {
    UpdateOscScrollList();

    static std::string myMdnsName = "";
    std::string otherName = oscDataSource->mDnsName;
    if (myMdnsName != otherName) {
        myMdnsName = otherName;
        mDnsNameText->set_text(SSL10nGen::STR::mdns_name_title() + myMdnsName);
    }
}