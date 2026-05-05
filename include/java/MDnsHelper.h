#pragma once

#include <string>
#include "java.h"
#include <jni.h>

namespace HeartBeat {

// clang-format off
enum MDnsIdSource{
    MDNS_ID_OSC = 2,
    MDNS_ID_HTTP_SERVER = 3
};
// clang-format on

class MDnsHelper : public JavaSingleClassObject {
private:
    // clang-format off
    jmethodID
        m_SetManagerId,
        m_SetMdnsName,
        m_Stop
        ;
    // clang-format on
public:
    MDnsHelper(jobject SomeClassLoader, jmethodID LoadClassMethod, JNIEnv* env)
        : JavaSingleClassObject(SomeClassLoader, LoadClassMethod, env, "top.zxff.nativeblereader.MDnsHelper") {
        m_SetManagerId = GetMethodID("SetManagerId", "(I)V");
        m_SetMdnsName = GetMethodID("SetMdnsName", "(Ljava/lang/String;ILjava/lang/String;)V");
        m_Stop = GetMethodID("Stop", "()V");

        nativeMethods.emplace_back("InformRegName", "(ILjava/lang/String;)Z", reinterpret_cast<void*>(OnInformRegName));
    }

    void SetManagerId(jint id) {
        env->CallVoidMethod(ThisObj, m_SetManagerId, id);
        CheckException();
    }
    void SetMDnsName(std::string name, jint port, std::string type) {
        env->CallVoidMethod(ThisObj, m_SetMdnsName, env->NewStringUTF(name.c_str()), port,
                            env->NewStringUTF(type.c_str()));
        CheckException();
    }

    void Stop() {
        env->CallVoidMethod(ThisObj, m_Stop);
        CheckException();
    }

    static JNIEXPORT void JNICALL OnInformRegName(JNIEnv* env, jobject thiz, jint id, jstring devName);

    static MDnsHelper* osc_instance;
};

} // namespace HeartBeat
