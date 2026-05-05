#pragma once

#include "DataSource.hpp"
#include <mutex>
#include <optional>
#include <string>
#include <map>

namespace HeartBeat {

enum BluetoothManifestPermission {
    BLE_MANI_PERM_UNKNOWN = 0,
    BLE_MANI_PERM_GOOD_LOCATION_REQUIRED = 1,
    BLE_MANI_PERM_GOOD_NONEED_LOCATION = 2,
    BLE_MANI_PERM_BAD_BLUETOOTH_OR_LOCATION_MISSED = 3,
    BLE_MANI_PERM_PAIRED_ONLY = 4,
};

struct HeartBeatBleDevice {
    std::string name;
    std::string mac;
    int last_data;
    int last_data_time;
};

class HeartBeatBleDataSource : public DataSource {
private:
    std::string selected_mac = "";
    std::mutex selected_mac_lock;

    bool has_new_data;
    int heartbeat = 0;
    std::atomic_llong energy = 0;
    std::atomic_llong persistent_energy = 0;
    bool is_auto_connecting = false;
    bool is_scanning = false;
    BluetoothManifestPermission manifestPermission;

public:
    HeartBeatBleDataSource();
    bool GetData(int& heartbeat) override;
    long long GetEnergy() override;

    void StartScan();
    void StopScan();
    void StartAutoScan();
    void SetAutoConnectPattern(const std::string& macAddr, const std::string& devName);

    void OpenSystemLocationSetthings();

    bool isScanning() { return is_scanning; }
    bool isAutoConnecting() { return is_auto_connecting; }


    std::string& GetSelectedBleMac() {
        std::lock_guard<std::mutex> g(selected_mac_lock);
        return selected_mac;
    }
    void SetSelectedBleMac(const std::string mac, std::optional<std::function<void(void)>> callback);
    BluetoothManifestPermission GetBleManifestPermissionStatus() { return manifestPermission; }

    std::map<std::string /*mac*/, HeartBeatBleDevice> avaliable_devices;

    // called from java
    bool InformNativeDevice(std::string macAddr, std::string name);
    void OnDataCome(const std::string& macAddr, int heartRate, long energy);
    void OnEnergyReset();
    void OnAutoConnectStatusChanged(bool autoConnecting);
    void OnScanStatusChanged(bool isScanning);

    static HeartBeatBleDataSource* bleDataSource;
};

} // namespace HeartBeat