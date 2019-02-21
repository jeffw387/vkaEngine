#pragma once
#include <chrono>

namespace vka {
using Clock = std::chrono::high_resolution_clock;
static constexpr Clock::duration OneSecond =
    std::chrono::seconds(1);
}  // namespace vka