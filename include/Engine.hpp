#pragma once
#include "VulkanFunctionLoader.hpp"
#include <array>
#include <chrono>
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "Instance.hpp"
#include "Surface.hpp"

namespace vka {
constexpr auto BufferCount = 3U;
using Clock = std::chrono::high_resolution_clock;
constexpr Clock::duration OneSecond = std::chrono::seconds(1);

class Instance;

class Engine : std::enable_shared_from_this<Engine> {
public:
  using Ptr = std::shared_ptr<Engine>;
  using UpdateCallback = std::function<void(Engine::Ptr)>;
  struct CreateInfo {
    UpdateCallback updateCallback;
    Surface::CreateInfo surfaceCreateInfo;
    Instance::CreateInfo instanceCreateInfo;
  };
  Engine(CreateInfo);
  void run();
  void setUpdatesPerSecond(uint32_t count) { updatesPerSecond = count; }

private:
  void initInputCallbacks(GLFWwindow* windowHandle);
  void renderThreadFunc();
  void handleOSMessages();
  void acquireUpdateSlot();
  void setLastUpdated(int32_t);
  void acquireRenderSlot();
  Clock::duration updateDuration() { return OneSecond / updatesPerSecond; }

  CreateInfo engineCreateInfo;
  std::shared_ptr<Instance> instance;
  std::mutex stateMutex;
  int32_t updateIndex = -1;
  int32_t lastUpdatedIndex = -1;
  int32_t renderIndex = -1;
  bool continueRendering;
  bool continueUpdating;
  uint32_t updatesPerSecond;
  std::array<Clock::time_point, BufferCount> indexUpdateTime;
  UpdateCallback updateCallback;
};
}  // namespace vka