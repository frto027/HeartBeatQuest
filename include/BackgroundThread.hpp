#pragma once

#include <functional>

namespace HeartBeat {
void runBackground(std::function<void(void)>);
void terminateBackground();
} // namespace HeartBeat