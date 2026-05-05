The `HeartBeatBLEReader.dex` file is compiled from `/AndroidProject/HeartBeatNative` and extracted from:
`AndroidProject\HeartBeatNative\nativeblereader\build\outputs\apk\release\nativeblereader-release.apk`

1. Build `AndroidProject/HeartBeatNative` with Android Studio.
2. Extract the APK, copy the `classes.dex` file, and rename it to `HeartBeatBLEReader.dex` here.
3. From the project root, run `python update_resources.py`.
4. Recompile the program.
