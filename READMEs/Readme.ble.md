## Bluetooth Device as the Data Source

This mod can access Bluetooth directly. To use it, follow these steps:

1. Patch the game with the `bluetooth` permission via [MBF](https://mbf.bsquest.xyz/) (the recommended way to mod Beat Saber).
2. Install this mod via MBF.
3. In your Quest's Bluetooth settings, pair your heart rate BLE device with your Quest.
4. Open the game, scan, and select your device in the device list menu.

The Bluetooth data source has minimal data latency, but may have less compatibility because it uses a generic BLE protocol. Your device should support heart rate broadcast over BLE.

> [!NOTE]
> Bluetooth permission is not required if you don't use this data source.

**Reconnect**  
If you have trouble with your current connection, select `None` in the device list to disconnect, then reselect your device to reconnect.

**Auto Reconnect**  
From version `0.4.0`, the mod will automatically scan for devices in the first 20 seconds after game start (if sufficient permission is granted). If it finds a device with the same MAC address as the last connection, it will try to auto-connect.  

Scanning consumes battery, so if your HR device is not available within this window, you'll need to manually start a scan from the device list.

**Manual Permission Patch**  
See the [Bluetooth Permission Guide](https://github.com/frto027/HeartBeatQuest/wiki/Bluetooth-Permission-Guide).
