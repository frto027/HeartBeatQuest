# Development information

## How it works for Bluetooth device

This mod uses [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html) to load an external java library with [PathClassLoader](https://developer.android.com/reference/dalvik/system/PathClassLoader) to access bluetooth device.

The Bluetooth access code at `AndroidProject/HeartBeatNative` can be compiled with AndroidStudio and Android SDK 34.

## The threads

refer to [Readme.thread.md](Readme.thread.md).

## How to build

**make sure you have cloned the submodules.**
```sh
git clone --recursive git@github.com:frto027/HeartBeatQuest.git
```

You need an environment variable `ANDROID_NDK_HOME` with proper value. Do NOT use `ndkpath.txt`.

mod build: `qpm restore && qpm s build`  
mod debug: `qpm s copy`  
mod release: `qpm qmod zip`  

This mod is avaliable for multiple game versions. See [multibuild](../multibuild) directory about switch build config for different game versions.

This mod's java code is hard coded into the C source. See [extra/Readme.md](../extra/Readme.md) if you want compile the java code.


## Mod api

### Basic API Usage

Copy `shared/HeartBeatApi.h` file to your mod. It is header-only, so you don't need a hard dependency.

The file is clean, and only depends on `scotland2` mod loader.

for example in your mod:

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
            //new data come in this frame
        }else{
            //old buffered data got
        }
        // the result of GetData will not chaned until you call heartBeatApi->Update() at next game update cycle
    }
}
```

### API ChangeLog

- `0.3.6` The mod id will be changed to `HeartBeatQuest` for game version larger than `v1.40.8`(not included). Add support for the new mod id.

