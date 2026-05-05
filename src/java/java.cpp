#include "java/java.h"
#include "data_sources/DataSource.hpp"
#include "java/MDnsHelper.h"
#include "java/ModHelper.h"
#include "main.hpp"
#include "scotland2/shared/modloader.h"
#include <jni.h>
#include <stdexcept>
#include "HeartBeatBLEDex.inl"
#include "java/BleReader.h"
#include "SettingsSnapshot.hpp"
#include "main.hpp"

HeartBeat::JavaSingleClassObject::JavaSingleClassObject(jobject SomeClassLoader, jmethodID LoadClassMethod, JNIEnv* env,
                                                        const char* packageName)
    : env(env) {
    // We can't use env->FindClass("top.zxff.nativeblereader.BleReader") to find our class
    // Because we use a different class loader, which is a InMemoryDexClassLoader,
    //      and FindClass uses the classloader that ralated to the top of call stack
    auto return_value_of_loadClass =
        env->CallObjectMethod(SomeClassLoader, LoadClassMethod, env->NewStringUTF(packageName));
    CheckException();
    ThisClass = static_cast<jclass>(return_value_of_loadClass);
    if (ThisClass == nullptr) {
        getLogger().error("Class not found.");
        throw std::runtime_error("jni class not found");
    }
    ctor = env->GetMethodID(ThisClass, "<init>", "()V");
    CheckException();
}

void HeartBeat::JavaSingleClassObject::CheckException() {
    if (env->ExceptionCheck()) {
        getLogger().error("JNI Exception at (line {})", __LINE__);
        env->ExceptionDescribe();
        env->ExceptionClear();
        throw std::runtime_error("JNI Exception occurred.");
    }
}

bool HeartBeat::JavaSingleClassObject::CheckExceptionSafe() {
    if (env->ExceptionCheck()) {
        getLogger().error("JNI Exception at (line {})", __LINE__);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}
void HeartBeat::JavaSingleClassObject::CreateObject() {
    if (!created) {
        created = true;
        ThisObj = env->NewGlobalRef(env->NewObject(ThisClass, ctor));
    }
}

void HeartBeat::JavaSingleClassObject::RegisterNatives() {
    int rc = env->RegisterNatives(ThisClass, nativeMethods.data(), nativeMethods.size());
    if (rc != JNI_OK) {
        getLogger().error("Failed to register native methods");
        return;
    }
}
jmethodID HeartBeat::JavaSingleClassObject::GetMethodID(const char* methodName, const char* signature) {
    auto ret = env->GetMethodID(ThisClass, methodName, signature);
    getLogger().info("Loading java method {}, result is {}", methodName, (void*)ret);
    return ret;
}

bool HeartBeat::LoadJavaLibraryIfNeeded() {
    switch (HeartBeat::SettingsSnapshot::getInstance()->DataSourceType) {
        case HeartBeat::DS_BLE:
        case HeartBeat::DS_OSC:
            break;
        default:
            return false;
    }

    JNIEnv* env;
    auto ret = modloader_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    if (env == nullptr) {
        getLogger().error("JNI Env is nullptr");
        return false;
    }
    env->ExceptionClear();

#define CHECK_EXCEPTION()                                                                                              \
    do {                                                                                                               \
        if (env->ExceptionCheck()) {                                                                                   \
            getLogger().error("JNI Exception at (line {})", __LINE__);                                                 \
            env->ExceptionDescribe();                                                                                  \
            env->ExceptionClear();                                                                                     \
            return false;                                                                                              \
        }                                                                                                              \
    } while (0)

    jobject ClassLoader;
    {
        jclass atClass = env->FindClass("android/app/ActivityThread");

        jmethodID currentApplication =
            env->GetStaticMethodID(atClass, "currentApplication", "()Landroid/app/Application;");

        jobject application = env->CallStaticObjectMethod(atClass, currentApplication);
        jclass contextClass = env->FindClass("android/content/Context");

        jmethodID getClassLoader = env->GetMethodID(contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

        ClassLoader = env->CallObjectMethod(application, getClassLoader);
    }
    // auto ClassLoader = env->CallObjectMethod(unityPlayerClass, ClassClass_getClassLoader);
    CHECK_EXCEPTION();

    jobject buffobj;
    {
        auto arr = env->NewByteArray(sizeof(ble_dex));
        env->SetByteArrayRegion(arr, 0, sizeof(ble_dex), (const jbyte*)&*ble_dex);

        CHECK_EXCEPTION();

        auto ByteBufferClass = env->FindClass("java/nio/ByteBuffer");
        auto ByteBufferClass_wrap = env->GetStaticMethodID(ByteBufferClass, "wrap", "([B)Ljava/nio/ByteBuffer;");
        buffobj = env->CallStaticObjectMethod(ByteBufferClass, ByteBufferClass_wrap, arr);
    }

    auto SomeClassLoaderClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    if (SomeClassLoaderClass == nullptr) {
        getLogger().error("can't find dalvik.system.InMemoryDexClassLoader");
        return false;
    }
    auto SomeClassLoaderInit =
        env->GetMethodID(SomeClassLoaderClass, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    if (SomeClassLoaderInit == nullptr) {
        getLogger().debug("Enpty LoaderInit");
        return false;
    }
    getLogger().debug("newObject");
    auto SomeClassLoader = env->NewObject(SomeClassLoaderClass, SomeClassLoaderInit, buffobj, ClassLoader);
    CHECK_EXCEPTION();
    getLogger().debug("will loadClass()");
    auto LoadClassMethod = env->GetMethodID(SomeClassLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    try {
        HeartBeat::JavaModHelper::instance = new JavaModHelper(SomeClassLoader, LoadClassMethod, env);

        if (HeartBeat::SettingsSnapshot::getInstance()->DataSourceType == DS_OSC) {
            HeartBeat::MDnsHelper::osc_instance = new MDnsHelper(SomeClassLoader, LoadClassMethod, env);
            HeartBeat::MDnsHelper::osc_instance->RegisterNatives();
        }

        if (HeartBeat::SettingsSnapshot::getInstance()->DataSourceType == DS_BLE) {
            HeartBeat::BleReader::instance = new BleReader(SomeClassLoader, LoadClassMethod, env);
            HeartBeat::BleReader::instance->RegisterNatives();
        }
    } catch (std::runtime_error err) {
        getLogger().error("Exception in java {}", err.what());
        return false;
    }
    return true;
}