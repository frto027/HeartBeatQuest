#pragma once

#ifdef WITH_QOUNTERS

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "rapidjson-macros/shared/macros.hpp"


// this types is copied from qounters mod

namespace Qounters::Types {
enum class Sources {
    Text,
    Shape,
    Color,
    Enable,
};
template <class T> using SourceFn = std::function<T(UnparsedJSON)>;
using SourceUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
using PremadeFn = std::function<UnityEngine::UI::Graphic*(UnityEngine::GameObject*, UnparsedJSON)>;
using PremadeUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
using PremadeUpdateFn = std::function<void(UnityEngine::UI::Graphic*, UnparsedJSON)>;
using TemplateUIFn = std::function<void(UnityEngine::GameObject*)>;
} // namespace Qounters::Types

namespace Qounters::Sources {
using TextsTy = std::vector<
    std::pair<std::string, std::pair<::Qounters::Types::SourceFn<std::string>, ::Qounters::Types::SourceUIFn>>>;
using ShapesTy = std::vector<std::pair<std::string, std::pair<Types::SourceFn<float>, Types::SourceUIFn>>>;
using ColorsTy = std::vector<std::pair<std::string, std::pair<Types::SourceFn<UnityEngine::Color>, Types::SourceUIFn>>>;
using EnablesTy = std::vector<std::pair<std::string, std::pair<Types::SourceFn<bool>, Types::SourceUIFn>>>;

template <class T>
inline void Register(std::vector<std::pair<std::string, std::pair<T, Types::SourceUIFn>>>& sourceVec,
                     std::string source, T sourceFn, Types::SourceUIFn sourceUIFn) {
    sourceVec.emplace_back(source, std::make_pair(sourceFn, sourceUIFn));
}
} // namespace Qounters::Sources

namespace HeartBeat {
namespace Qounters {
void Init();
void DisplayData(int heartrate);

bool Enabled();
} // namespace Qounters
} // namespace HeartBeat

#endif // WITH_QOUNTERS