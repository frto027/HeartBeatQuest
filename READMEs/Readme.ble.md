## Bluetooth device as the data source

This mod can access Bluetooth directly. To use this, follow this instruction.

1. patch the game with the `bluetooth` permission via [mbf](https://mbf.bsquest.xyz/), which is the recommand way to mod the beatsaber now.
1. install this mod via mbf.
2. In your quest bluetooth setthings, pair your heart rate BLE device with your quest.
3. Open the game, scan and select your device in the device list menu.

The Bluetooth data source has minimum data latency, but may be less compatibility because it uses a generic BLE protocol to access Bluetooth devices. Your device should support heart rate broadcast via BLE protocol.

> [!NOTE]
> Bluetooth permission is not required if you don't use this data source.

**Reconnect**
In case you have trouble with your current connection. If you select `None` in the device list, your device will be disconnected.
Then you can reselect your device to reconnect it.

**Auto Reconnect**  
From version `0.4.0`, the mod will auto scan devices in the first 20 seconds when game start if there's enough permission.
If it find any device that has same mac address with last connect, the mod will try to auto connect that device.  
Scan will consume your quest's battery, so if your hr device is not avaliable in this time window, you need manually start scan in the device list.

**Manual Permission Patch**  
Please refer to [Bluetooth Permission Guide](https://github.com/frto027/HeartBeatQuest/wiki/Bluetooth-Permission-Guide)


