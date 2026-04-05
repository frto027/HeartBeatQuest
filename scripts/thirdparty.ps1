# build libcurl manually
cmake -B build_third_party/libhv -S third_party/libhv -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$ENV:ANDROID_NDK_HOME\build\cmake\android.toolchain.cmake" -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-32 -DCMAKE_BUILD_TYPE=Release
pushd build_third_party/libhv
ninja
popd