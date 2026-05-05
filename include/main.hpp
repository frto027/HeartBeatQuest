#pragma once

#include "multi_version_compat.hpp"
#include <string>

namespace HeartBeat {
class HeartBeatObj;
}
Paper::ConstLoggerContext<21>& getLogger();

extern std::string modConfigFilePath;
