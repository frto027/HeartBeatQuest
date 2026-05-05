# HeartBeatQuest (BeatSaber mod) 

<img src="cover.png" align="right" width="300">

[![QMOD BUILD](https://github.com/frto027/HeartBeatQuest/actions/workflows/qmod_build.yml/badge.svg)](https://github.com/frto027/HeartBeatQuest/actions/workflows/qmod_build.yml) ![GitHub Release](https://img.shields.io/github/v/release/frto027/HeartBeatQuest?include_prereleases)

> [!TIP]
> This mod is for the Quest platform. For PC, use [HRCounter](https://github.com/qe201020335/HRCounter). 

[中文简要说明](./READMEs/README.cn.md)

View your heart rate inside Beat Saber on Quest.

# Brief Usage

After patching the mod (e.g. with [MBF](https://mbf.bsquest.xyz/), with or without `bluetooth` permission), you can configure it in-game via a button on your left side. Change the data source and restart the game, and it works.

> [!NOTE]
> **HEART RATE RECORD**  
> This mod will automatically record your heart rate to BeatLeader's replay file if BeatLeader-qmod is detected. You can disable this feature in the mod settings.

> [!TIP]
> **MOD ROADMAP**  
> Suggestions are welcome — feel free to open an issue!

# Documentation

[Bluetooth permission guide for MBF](READMEs/ModsBeforeFridayGuide/BLE.md)

[Data source configuration](READMEs/Readme.datasource.md)

[Mod skin document](READMEs/Readme.skin.md)

[Collaboration mod document](READMEs/Readme.co-mods.md)

[Custom UI document](READMEs/Readme.ui.md)

[Development document](READMEs/Readme.develop.md)

# Other Information

For version-independent information (replay data format, supported game versions, etc.), please refer to the [wiki](https://github.com/frto027/HeartBeatQuest/wiki).

# Credits

Created by frto027.

Thanks to everyone who has directly or indirectly supported this mod:

* [zoller27osu](https://github.com/zoller27osu), [Sc2ad](https://github.com/Sc2ad) and [jakibaki](https://github.com/jakibaki) — [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook)
* [raftario](https://github.com/raftario)
* [Lauriethefish](https://github.com/Lauriethefish), [danrouse](https://github.com/danrouse) and [Bobby Shmurner](https://github.com/BobbyShmurner) for [this template](https://github.com/Lauriethefish/quest-mod-template)
* NSGolova — [BeatLeader](https://github.com/BeatLeader/beatleader-qmod) for replay and webreplay support
* And other developers in the BSMG Discord channel
* [Hyperate](https://www.hyperate.io) — API support
* [Pulsoid](https://pulsoid.net/) — API support
* [IXWebSocket](https://github.com/machinezone/IXWebSocket) — websocket/http client
