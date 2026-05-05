#include "java/BleReader.h"
#include "data_sources/Bluetooth.hpp"

JNIEXPORT void JNICALL HeartBeat::BleReader::OnDeviceData(JNIEnv* env, jobject thiz, jstring macAddr, jint heartRate,
                                                          jlong energy) {
    // this happens on a background thread, as google documented
    auto chars = env->GetStringUTFChars(macAddr, NULL);
    HeartBeat::HeartBeatBleDataSource::bleDataSource->OnDataCome(chars, heartRate, energy);
    env->ReleaseStringUTFChars(macAddr, chars);
}
JNIEXPORT jboolean JNICALL HeartBeat::BleReader::InformNativeDevice(JNIEnv* env, jobject thiz, jstring macAddr,
                                                                    jbyteArray name) {
    // Add the ui or do something...
    // bleReader_BleStart call this function in java code
    auto macChar = env->GetStringUTFChars(macAddr, NULL);

    jbyte* name_buff = env->GetByteArrayElements(name, NULL);
    jsize name_len = env->GetArrayLength(name);
    std::string name_str((char*)name_buff, (size_t)name_len);

    jboolean ret = HeartBeat::HeartBeatBleDataSource::bleDataSource->InformNativeDevice(macChar, name_str);
    env->ReleaseStringUTFChars(macAddr, macChar);
    env->ReleaseByteArrayElements(name, name_buff, 0);
    return ret;
}
JNIEXPORT void JNICALL HeartBeat::BleReader::OnEnergyReset(JNIEnv* env, jobject thiz) {
    HeartBeat::HeartBeatBleDataSource::bleDataSource->OnEnergyReset();
}
JNIEXPORT void JNICALL HeartBeat::BleReader::OnAutoConnectStatusChanged(JNIEnv* env, jobject thiz,
                                                                        jboolean autoConnecting) {
    HeartBeat::HeartBeatBleDataSource::bleDataSource->OnAutoConnectStatusChanged(autoConnecting);
}
JNIEXPORT void JNICALL HeartBeat::BleReader::OnScanStatusChanged(JNIEnv* env, jobject thiz, jboolean isScanning) {
    HeartBeat::HeartBeatBleDataSource::bleDataSource->OnScanStatusChanged(isScanning);
}

HeartBeat::BleReader* HeartBeat::BleReader::instance = nullptr;