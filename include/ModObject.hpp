#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"

#include <functional>

// parameters are (namespace, class name, parent class, contents)
#if defined(GAME_VER_1_28_0) || defined(GAME_VER_1_35_0) || defined(GAME_VER_1_37_0)
DECLARE_CLASS_CODEGEN(HeartBeat, ModObject, UnityEngine::MonoBehaviour,
#else
DECLARE_CLASS_CODEGEN(HeartBeat, ModObject, UnityEngine::MonoBehaviour){
#endif
    // DECLARE_INSTANCE_METHOD creates methods
    DECLARE_INSTANCE_METHOD(void, Start); // will called when create
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnApplicationQuit); // do some clean up if we want

#if defined(GAME_VER_1_28_0) || defined(GAME_VER_1_35_0) || defined(GAME_VER_1_37_0)
);
#else
};
#endif

namespace HeartBeat{

    void InitModObject();
    void runInUnityThread(std::function<void(void)>);

}
