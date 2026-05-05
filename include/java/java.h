#pragma once

#include <jni.h>
#include <stdexcept>
#include <vector>

namespace HeartBeat {

bool LoadJavaLibraryIfNeeded();


class JavaSingleClassObject {
private:
    jclass ThisClass;
    jmethodID ctor;

public:
    jobject ThisObj = nullptr;
    JavaSingleClassObject(jobject SomeClassLoader, jmethodID LoadClassMethod, JNIEnv* env, const char* packageName);

    void CreateObject();

    void RegisterNatives();

protected:
    JNIEnv* env;
    bool created = false;

    std::vector<JNINativeMethod> nativeMethods;
    jmethodID GetMethodID(const char* methodName, const char* signature);
    void CheckException();
    bool CheckExceptionSafe();
};
} // namespace HeartBeat