# Development Information

## Bluetooth Device Support

This mod uses [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html) to load an external Java library via [PathClassLoader](https://developer.android.com/reference/dalvik/system/PathClassLoader) for Bluetooth access.

The Bluetooth code at `AndroidProject/HeartBeatNative` can be compiled with Android Studio and Android SDK 34.

## Thread Model

See [Readme.thread.md](Readme.thread.md).

## How to Build

**Make sure you have cloned the submodules:**
```sh
git clone --recursive git@github.com:frto027/HeartBeatQuest.git
```

Set the `ANDROID_NDK_HOME` environment variable. Do **not** use `ndkpath.txt`.

- Build: `qpm restore && qpm s build`
- Debug: `qpm s copy`
- Release: `qpm qmod zip`

This mod supports multiple game versions. See the [multibuild](../multibuild) directory for switching build configs between versions.

The mod's Java code is compiled into the C source. See [extra/Readme.md](../extra/Readme.md) if you need to compile the Java code.

## Mod API

### Basic API Usage

Copy `shared/HeartBeatApi.h` into your mod. It is header-only, so no hard dependency is needed.

The file is clean and only depends on the `scotland2` mod loader.

For example, in your mod:

```cpp
#include "HeartBeatApi.h"

HeartBeatApi * heartBeatApi = nullptr;
extern "C" void late_load() {
    heartBeatApi = HeartBeat::GetHeartBeatApi();
}

void Update(){
    if(heartBeatApi){
        heartBeatApi->Update();
        int data;
        if(heartBeatApi->GetData(&data)){
            // new data came in this frame
        }else{
            // old buffered data retrieved
        }
        // The result of GetData() will not change until
        // you call heartBeatApi->Update() at the next game update cycle.
    }
}
```

### API Changelog

- `0.3.6` — The mod ID will change to `HeartBeatQuest` for game versions greater than `v1.40.8` (not inclusive). Added support for the new mod ID.
