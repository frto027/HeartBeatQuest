#pragma once

#include <string>
#include "java.h"

namespace HeartBeat {

class BleReader : public JavaSingleClassObject {
private:
    // clang-format off
    jmethodID
        m_bleToggle,
        m_isDeviceSelected,
        m_bleScanStart,
        m_bleScanStop,
        m_autoConnectStart,
        m_autoConnectStop,
        m_autoConnectSetPattern,
        m_openSystemLocationSetthings,
        m_getPermisionStatus
        ;
    // clang-format on
public:
    BleReader(jobject SomeClassLoader, jmethodID LoadClassMethod, JNIEnv* env)
        : JavaSingleClassObject(SomeClassLoader, LoadClassMethod, env, "top.zxff.nativeblereader.BleReader") {
        nativeMethods.emplace_back("OnDeviceData", "(Ljava/lang/String;IJ)V", reinterpret_cast<void*>(OnDeviceData));
        nativeMethods.emplace_back("InformNativeDevice", "(Ljava/lang/String;[B)Z",
                                   reinterpret_cast<void*>(InformNativeDevice));
        nativeMethods.emplace_back("OnEnergyReset", "()V", reinterpret_cast<void*>(OnEnergyReset));
        nativeMethods.emplace_back("OnAutoConnectStatusChanged", "(Z)V",
                                   reinterpret_cast<void*>(OnAutoConnectStatusChanged));
        nativeMethods.emplace_back("OnScanStatusChanged", "(Z)V", reinterpret_cast<void*>(OnScanStatusChanged));

        m_bleToggle = GetMethodID("BleToggle", "(Ljava/lang/String;Z)Z");
        m_isDeviceSelected = GetMethodID("IsDeviceSelected", "(Ljava/lang/String;)Z");
        m_bleScanStart = GetMethodID("BleScanStart", "()V");
        m_bleScanStop = GetMethodID("BleScanStop", "()V");
        m_autoConnectStart = GetMethodID("AutoConnectStart", "()V");
        m_autoConnectStop = GetMethodID("AutoConnectStop", "()V");
        m_autoConnectSetPattern = GetMethodID("AutoConnectSetPattern", "(Ljava/lang/String;Ljava/lang/String;)V");
        m_openSystemLocationSetthings = GetMethodID("OpenSystemLocationSetthing", "()V");
        m_getPermisionStatus = GetMethodID("getPermisionStatus", "()I");
    }

    bool bleToggle(std::string macAddr, bool selected) {
        auto ret = env->CallBooleanMethod(ThisObj, m_bleToggle, env->NewStringUTF(macAddr.c_str()), selected);
        CheckException();
        return ret;
    }
    bool isDeviceSelected(std::string macAddr) {
        auto ret = env->CallBooleanMethod(ThisObj, m_bleToggle, env->NewStringUTF(macAddr.c_str()));
        CheckException();
        return ret;
    }
    void bleScanStart() {
        env->CallVoidMethod(ThisObj, m_bleScanStart);
        CheckException();
    }
    void bleScanStop() {
        env->CallVoidMethod(ThisObj, m_bleScanStop);
        CheckException();
    }
    void autoConnectStart() {
        env->CallVoidMethod(ThisObj, m_autoConnectStart);
        CheckException();
    }
    void autoConnectStop() {
        env->CallVoidMethod(ThisObj, m_autoConnectStop);
        CheckException();
    }
    void autoConnectSetPattern(const std::string& macAddr, const std::string& devName) {
        env->CallVoidMethod(ThisObj, m_autoConnectSetPattern, env->NewStringUTF(macAddr.c_str()),
                            env->NewStringUTF(devName.c_str()));
        CheckException();
    }
    void openSystemLocationSetthings() {
        env->CallVoidMethod(ThisObj, m_openSystemLocationSetthings);
        CheckException();
    }
    int getPermisionStatus() {
        auto ret = env->CallIntMethod(ThisObj, m_getPermisionStatus);
        CheckException();
        return ret;
    }

    static JNIEXPORT void JNICALL OnDeviceData(JNIEnv* env, jobject thiz, jstring macAddr, jint heartRate,
                                               jlong energy);
    static JNIEXPORT jboolean JNICALL InformNativeDevice(JNIEnv* env, jobject thiz, jstring macAddr, jbyteArray name);
    static JNIEXPORT void JNICALL OnEnergyReset(JNIEnv* env, jobject thiz);
    static JNIEXPORT void JNICALL OnAutoConnectStatusChanged(JNIEnv* env, jobject thiz, jboolean autoConnecting);
    static JNIEXPORT void JNICALL OnScanStatusChanged(JNIEnv* env, jobject thiz, jboolean isScanning);

    static BleReader* instance;
};

} // namespace HeartBeat
