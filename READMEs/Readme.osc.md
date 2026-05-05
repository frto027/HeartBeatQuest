## OSC as the Heart Data Source

Use your favorite heart rate OSC sender and send data to port 9000 on your Quest device. **If your sender is not supported, please [let me know](https://github.com/frto027/HeartBeatQuest/issues).**

After switching to OSC in the settings menu, the port will be shown in the menu. You can edit it manually via the config file if needed.

This mod supports mDNS. With this feature, you can use `osc.heartbeatquest.local` instead of your Quest's IP address. Enable this in the OSC settings menu. The hostname can also be edited via the config file.
