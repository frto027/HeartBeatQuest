#include "java/MDnsHelper.h"
#include "ModObject.hpp"
#include "data_sources/OSC.hpp"

JNIEXPORT void JNICALL HeartBeat::MDnsHelper::OnInformRegName(JNIEnv* env, jobject thiz, jint id, jstring devName) {
    if (id == MDNS_ID_OSC) {
        auto chars = env->GetStringUTFChars(devName, NULL);
        std::string devNameStr = chars;
        env->ReleaseStringUTFChars(devName, chars);

        runInUnityThread([devNameStr]() {
            HeartBeat::DataSource::getInstance()->as<HeartBeat::HeartBeatOSCDataSource>()->OnMDnsDevNameChanged(
                devNameStr);
        });
    }
}

HeartBeat::MDnsHelper* HeartBeat::MDnsHelper::osc_instance = nullptr;
