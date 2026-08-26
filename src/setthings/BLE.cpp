#include "ModObject.hpp"
#include "UnityEngine/Vector2.hpp"
#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "data_sources/Bluetooth.hpp"
#include "settings/BleSettings.hpp"
#include "settings/Settings.hpp"
#include <functional>

void HeartBeat::BleSettings::CreateElements() {
    auto *container = BSML::Lite::CreateVerticalLayoutGroup(controller->get_transform());
    bleDataSource = HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatBleDataSource>();

    auto *scanBtnContainer = BSML::Lite::CreateHorizontalLayoutGroup(container->get_transform());

    BSML::Lite::CreateUIButton(scanBtnContainer->get_transform(), SSL10nGen::STR::scan_start(), UnityEngine::Vector2{},
                               UnityEngine::Vector2{25, 8}, [this]() { bleDataSource->StartScan(); });
    BSML::Lite::CreateUIButton(scanBtnContainer->get_transform(), SSL10nGen::STR::scan_stop(), UnityEngine::Vector2{},
                               UnityEngine::Vector2{25, 8}, [this]() { bleDataSource->StopScan(); });

    switch (bleDataSource->GetBleManifestPermissionStatus()) {
    case BLE_MANI_PERM_UNKNOWN:
        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::ble_permission_unknown(),
                               UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8});
        break;
    case BLE_MANI_PERM_BAD_BLUETOOTH_OR_LOCATION_MISSED:
        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::ble_permission_needed(),
                               UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8});
        break;
    case BLE_MANI_PERM_GOOD_LOCATION_REQUIRED:
        BSML::Lite::CreateUIButton(container->get_transform(), SSL10nGen::STR::turn_location_on(),
                                   UnityEngine::Vector2{}, UnityEngine::Vector2{70, 8},
                                   [this]() { OpenWebpage("https://www.meta.com/help/quest/1202271140482151/"); });
    case BLE_MANI_PERM_GOOD_NONEED_LOCATION:
        // Fine permission manifest
        break;
    case HeartBeat::BLE_MANI_PERM_PAIRED_ONLY:
        BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::ble_permission_paired_only(),
                               UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8});
        break;
    }

    scanStatusText = BSML::Lite::CreateText(container->get_transform(), SSL10nGen::STR::no_scan(),
                                            UnityEngine::Vector2{}, UnityEngine::Vector2{50, 8});

    ble_list = BSML::Lite::CreateScrollableList(container->get_transform(), {70, 60},
                                                [this](int idx) { UpdateSelectedBLEValue(idx); });
    ble_list->set_listStyle(BSML::CustomListTableData::ListStyle::Simple);
    ble_list->tableView->set_selectionType(HMUI::TableViewSelectionType::Single);
    UpdateSelectedBLEScrollList();
}

void HeartBeat::BleSettings::Open() {
    // bleDataSource->StartScan();
}
void HeartBeat::BleSettings::Close() { bleDataSource->StopScan(); }
void HeartBeat::BleSettings::Update() {
    static int slow_down = 0;
    if (slow_down++ % 5 == 0) {
        UpdateSelectedBLEScrollList();
    }
    scanStatusText->set_text(bleDataSource->isAutoConnecting() ? SSL10nGen::STR::auto_scaning()
                             : bleDataSource->isScanning()     ? SSL10nGen::STR::scaning()
                                                               : SSL10nGen::STR::no_scan());
}

DEFINE_TYPE(HeartBeat, BluetoothDeviceItem);
HeartBeat::BluetoothDeviceItem *HeartBeat::BluetoothDeviceItem::construct() {
    auto ret = BluetoothDeviceItem::New_ctor();
    ret->text = "";
    ret->subText = nullptr;
    ret->icon = nullptr;
    ret->m_private_ui = private_ui;
    return ret;
}

void HeartBeat::BleSettings::UpdateSelectedBLEScrollList() {
    bool changed = false;

    std::string selected = bleDataSource->GetSelectedBleMac();

    if (ble_list->data.size() == 0) {
        auto dev = BluetoothDeviceItem::construct();
        dev->isNone = true;
        ble_list->data.push_back(dev);
        changed = true;
    }

    int select_cell_idx = -1;
    int index = 1;

    BluetoothDeviceItem *noneItem = (BluetoothDeviceItem *)ble_list->data->get_Item(0);
    if (selected == "")
        select_cell_idx = 0;
    changed |= noneItem->Update("", "", selected == "");

    for (auto dev : bleDataSource->avaliable_devices) {
        BluetoothDeviceItem *listItem;
        if (ble_list->data.size() <= index) {
            listItem = BluetoothDeviceItem::construct();
            ble_list->data.push_back(listItem);
            changed = true;
        } else {
            listItem = (BluetoothDeviceItem *)ble_list->data->get_Item(index);
        }

        bool isSelected = selected == dev.second.mac;
        changed |= listItem->Update(dev.second.name, dev.second.mac, isSelected);
        if (isSelected)
            select_cell_idx = index;
        index++;
    }
    if (index < ble_list->data.size()) {
        ble_list->data->RemoveRange(index, ble_list->data.size() - index);
        changed = true;
    }
    if (changed) {
        ble_list->tableView->ReloadData();
    }

    if (select_cell_idx >= 0) {
        ble_list->tableView->SelectCellWithIdx(select_cell_idx, false);
    }
}
void HeartBeat::BleSettings::UpdateSelectedBLEValue(int idx) {
    bleDataSource->SetSelectedBleMac(((BluetoothDeviceItem *)ble_list->data->get_Item(idx))->devMac, [this]() {
        runInUnityThread(std::bind(&BleSettings::UpdateSelectedBLEScrollList, this));
    });
}
