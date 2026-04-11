# build libcurl manually
cmake -B build_third_party/ixwebsocket -S third_party/IXWebSocket -G Ninja -DCMAKE_INSTALL_PREFIX=build_third_party/ixwebsocket -DUSE_ZLIB=OFF "-DCMAKE_TOOLCHAIN_FILE=$ENV:ANDROID_NDK_HOME\build\cmake\android.toolchain.cmake" -DCMAKE_CXX_FLAGS="-w" -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 -DCMAKE_BUILD_TYPE=Release
pushd build_third_party/ixwebsocket
ninja install
popd