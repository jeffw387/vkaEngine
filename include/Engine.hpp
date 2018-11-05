#pragma once
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "Logger.hpp"

namespace vka {
static constexpr auto BufferCount = 3U;
using Clock = std::chrono::high_resolution_clock;
static constexpr Clock::duration OneSecond = std::chrono::seconds(1);

class Engine;
class Instance;
struct InstanceCreateInfo;

struct GLFWOwner {
  GLFWOwner() {
    glfwInit();
    glfwSetErrorCallback([](int error, const char* desc) {
      MultiLogger::get()->error("GLFW error {}: {}", error, desc);
    });
  }
  GLFWOwner(GLFWOwner&&) = default;
  GLFWOwner& operator=(GLFWOwner&&) = default;
  GLFWOwner(const GLFWOwner&) = delete;
  GLFWOwner& operator=(const GLFWOwner&) = delete;
  ~GLFWOwner() { glfwTerminate(); }
};

using InitCallback = std::function<void(Engine*, int32_t)>;
using UpdateCallback = std::function<void(Engine*)>;
using RenderCallback = std::function<void(Engine*)>;
struct EngineCreateInfo {
  InitCallback initCallback;
  UpdateCallback updateCallback;
  RenderCallback renderCallback;
};

class Engine {
public:
  Engine() = default;
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(EngineCreateInfo);
  ~Engine() = default;

  Instance* createInstance(InstanceCreateInfo);
  Instance* getInstance() const noexcept { return instance.get(); }
  void run();
  void setUpdatesPerSecond(uint32_t count) { updatesPerSecond = count; }
  int32_t previousUpdateIndex() const noexcept { return lastUpdatedIndex; }
  int32_t currentUpdateIndex() const noexcept { return updateIndex; }
  int32_t currentRenderIndex() const noexcept { return renderIndex; }

private:
  void initInputCallbacks();
  void renderThreadFunc();
  void handleOSMessages();
  void acquireUpdateSlot();
  void setLastUpdated(int32_t);
  void acquireRenderSlot();
  Clock::duration updateDuration() { return OneSecond / updatesPerSecond; }

  GLFWOwner glfwOwner;
  std::unique_ptr<Instance> instance;
  std::mutex stateMutex;
  bool running = false;
  int32_t updateIndex = -1;
  int32_t lastUpdatedIndex = -1;
  int32_t renderIndex = -1;
  bool continueRendering;
  bool continueUpdating;
  uint32_t updatesPerSecond = 60;
  Clock::time_point startTime;
  std::array<Clock::time_point, BufferCount> indexUpdateTime;
  InitCallback initCallback;
  UpdateCallback updateCallback;
  RenderCallback renderCallback;
};
}  // namespace vka