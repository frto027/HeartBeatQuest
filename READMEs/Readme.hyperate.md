## HypeRate as the heart data source

This mod supports the [HypeRate](https://www.hyperate.io/) as the data source. Just install the mod and change the data source to HypeRate in the setthings menu, then restart the game. Input your hyperate ID in the HYPERATE menu and it will works. (Special thanks to HypeRate for providing API support)

> [!NOTE]
> **ONLINE DATA COLLECTION**  
> This is an online service, which means I need collect some game information to make sure the service has no problem, such as [the version number](https://github.com/frto027/HeartBeatQuest/blob/4243eadcc4062ee619a6606da65a1ba4d50d91c8/src/HBHeartBeatHypeRateDataSource.cpp#L327). This data is only used to check for service errors and is usually automatically deleted within 3 days. If you don't want send these data to server, you can use other data sources. If you have trouble, please make sure your quest device is able to access [this website](https://heart.0xf7.top/).
>
> This mod use Cloudflare as the super cool and fast backend.
