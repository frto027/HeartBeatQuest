#include <cstddef>
#include <jni.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include "BackgroundThread.hpp"
#include "ModObject.hpp"
#include "main.hpp"
#include "BeatLeaderRecorder.hpp"
#include "multi_version_compat.hpp"

#include "java/BleReader.h"
#include "data_sources/Bluetooth.hpp"
#include "ModConfig.hpp"

HeartBeat::HeartBeatBleDataSource* HeartBeat::HeartBeatBleDataSource::bleDataSource;

void StartScanDevice() {
    try {
        HeartBeat::BleReader::instance->bleScanStart();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}

bool ToggleDevice(std::string macAddr, jboolean selected) {
    getLogger().info("Toggle device to:|{}|", macAddr);
    try {
        return HeartBeat::BleReader::instance->bleToggle(macAddr, selected);
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
        return false;
    }
}

bool IsDeviceSelected(std::string macAddr) {
    try {
        return HeartBeat::BleReader::instance->isDeviceSelected(macAddr);
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
        return false;
    }
}
void StopScanDevice() {
    try {
        return HeartBeat::BleReader::instance->bleScanStop();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}
void AutoConnectStart() {
    try {
        return HeartBeat::BleReader::instance->autoConnectStart();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}
void AutoConnectStop() {
    try {
        return HeartBeat::BleReader::instance->autoConnectStop();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}
void AutoConnectSetPattern(const std::string& macAddr, const std::string& devName) {
    try {
        return HeartBeat::BleReader::instance->autoConnectSetPattern(macAddr, devName);
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}
void OpenSystemLocationSetthing() {
    try {
        return HeartBeat::BleReader::instance->openSystemLocationSetthings();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
    }
}
int GetPermisionStatus() {
    try {
        return HeartBeat::BleReader::instance->getPermisionStatus();
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
        return 0;
    }
}

HeartBeat::HeartBeatBleDataSource::HeartBeatBleDataSource()
    : HeartBeat::DataSource(HeartBeat::DataSourceType::DS_BLE) {
    bleDataSource = this;

    HeartBeat::BleReader::instance->CreateObject();
    StartAutoScan();
    manifestPermission = (BluetoothManifestPermission)GetPermisionStatus();
}

void HeartBeat::HeartBeatBleDataSource::SetSelectedBleMac(const std::string mac,
                                                          std::optional<std::function<void(void)>> callback) {
    runInUnityThread([this, mac, callback]() {
        ToggleDevice(GetSelectedBleMac(), false);
        {
            std::lock_guard<std::mutex> g(selected_mac_lock);
            selected_mac = mac;
        }
        getModConfig().SelectedBleMac.SetValue(mac, true);
        ToggleDevice(mac, true);

        auto it = avaliable_devices.find(this->selected_mac);
        if (it != avaliable_devices.end()) {
            Recorder::SetHeartDeviceName(it->second.name);
        } else {
            Recorder::SetHeartDeviceName(HEART_DEV_NAME_UNK);
        }
        if (callback.has_value()) {
            callback.value()();
        }
    });
}

void HeartBeat::HeartBeatBleDataSource::StartScan() {
    avaliable_devices.clear();
    StartScanDevice();
}
void HeartBeat::HeartBeatBleDataSource::StopScan() {
    StopScanDevice();
}
void HeartBeat::HeartBeatBleDataSource::StartAutoScan() {
    AutoConnectStart();
}
void HeartBeat::HeartBeatBleDataSource::SetAutoConnectPattern(const std::string& macAddr, const std::string& devName) {
    AutoConnectSetPattern(macAddr, devName);
}
void HeartBeat::HeartBeatBleDataSource::OpenSystemLocationSetthings() {
    OpenSystemLocationSetthing();
}
bool HeartBeat::HeartBeatBleDataSource::GetData(int& heartbeat) {
    heartbeat = this->heartbeat;
    if (has_new_data) {
        has_new_data = false;
        return true;
    }
    return false;
}

long long HeartBeat::HeartBeatBleDataSource::GetEnergy() {
    return this->energy.load() + this->persistent_energy.load();
}

bool HeartBeat::HeartBeatBleDataSource::InformNativeDevice(std::string macAddr, std::string name) {
    runInUnityThread([this, macAddr, name]() {
        if (avaliable_devices.find(macAddr) == avaliable_devices.end()) {
            avaliable_devices.insert({macAddr, {.name = name, .mac = macAddr, .last_data = 0, .last_data_time = 0}});
        }
    });

    std::string selected_mac = GetSelectedBleMac();

    if (selected_mac == "" && getModConfig().SelectedBleMac.GetValue() == macAddr) {
        SetSelectedBleMac(macAddr, {});
        Recorder::SetHeartDeviceName(name);
        return true;
    } else if (selected_mac == macAddr) {
        Recorder::SetHeartDeviceName(name);
        return true;
    } else {
        return false;
    }
}
void HeartBeat::HeartBeatBleDataSource::OnDataCome(const std::string& macAddr, int heartRate, long energy) {
    this->heartbeat = heartRate;
    this->has_new_data = true;
    this->energy.store(energy); // energy is not work, idk how to read the data from java code. just forget it.
}
void HeartBeat::HeartBeatBleDataSource::OnEnergyReset() {
    this->persistent_energy.fetch_add(this->energy.load());
    this->energy.store(0);
}
void HeartBeat::HeartBeatBleDataSource::OnAutoConnectStatusChanged(bool autoConnecting) {
    this->is_auto_connecting = autoConnecting;
}
void HeartBeat::HeartBeatBleDataSource::OnScanStatusChanged(bool isScanning) {
    this->is_scanning = isScanning;
}
