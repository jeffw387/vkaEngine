#pragma once
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Clock.hpp"
#include "Logger.hpp"
#include "Input.hpp"

namespace vka {
static constexpr auto BufferCount = 3U;

class Engine;
class Surface;
class Instance;
struct InstanceCreateInfo;

struct GLFW {
  GLFW() {
    MultiLogger::get()->info("Initializing GLFW");
    glfwInit();
    glfwSetErrorCallback([](int error, const char* desc) {
      MultiLogger::get()->error("GLFW error {}: {}", error, desc);
    });
  }
  ~GLFW() {
    MultiLogger::get()->info("Terminating GLFW");
    glfwTerminate();
  }
};

using UpdateCallback = std::function<void()>;
using RenderCallback = std::function<void()>;
struct EngineCreateInfo {
  UpdateCallback updateCallback;
  RenderCallback renderCallback;
};

class Engine {
public:
  Engine(EngineCreateInfo);

  std::unique_ptr<Instance> createInstance(InstanceCreateInfo);
  void run();
  void setUpdatesPerSecond(uint32_t count) { updatesPerSecond = count; }
  int32_t previousUpdateIndex() const noexcept { return lastUpdatedIndex; }
  int32_t currentUpdateIndex() const noexcept { return updateIndex; }
  int32_t currentRenderIndex() const noexcept { return renderIndex; }
  Clock::time_point updateTimePoint(int32_t index) const {
    return updateTimes[index];
  }
  void stop() {
    std::lock_guard<std::mutex> stateLock{stateMutex};
    continueUpdating = false;
    continueRendering = false;
  }

  Input::Manager inputManager;
  double mouseX;
  double mouseY;

private:
  void renderThreadFunc();
  void acquireUpdateSlot();
  void markStateUpdated(int32_t index, Clock::time_point updateTime);
  void acquireRenderSlot();
  Clock::duration updateDuration() { return OneSecond / updatesPerSecond; }

  std::unique_ptr<GLFW> glfwInstance;
  std::mutex stateMutex;
  int32_t updateIndex = -1;
  int32_t lastUpdatedIndex = -1;
  int32_t renderIndex = -1;
  bool continueRendering;
  bool continueUpdating;
  uint32_t updatesPerSecond = 60;
  Clock::time_point startTime;
  Clock::time_point updateTime;
  std::array<Clock::time_point, BufferCount> updateTimes;
  UpdateCallback updateCallback;
  RenderCallback renderCallback;
};
}  // namespace vka