#pragma once

#include "java.h"

namespace HeartBeat {

class JavaModHelper : public JavaSingleClassObject {
private:
    jmethodID m_onModExit;

public:
    JavaModHelper(jobject SomeClassLoader, jmethodID LoadClassMethod, JNIEnv* env)
        : JavaSingleClassObject(SomeClassLoader, LoadClassMethod, env, "top.zxff.nativeblereader.ModHelper") {
        m_onModExit = GetMethodID("OnModExit", "()V");
    }

    void OnModExit() {
        auto ret = env->CallIntMethod(ThisObj, m_onModExit);
        CheckException();
    }

    static JavaModHelper* instance;
};

} // namespace HeartBeat
