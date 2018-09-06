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
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace vka {
static constexpr auto BufferCount = 3U;
using Clock = std::chrono::high_resolution_clock;
static constexpr Clock::duration OneSecond = std::chrono::seconds(1);
static auto LogFileName = "vkaEngineLog.txt";
static auto LoggerName = "MultiLogger";

class Engine;
class Instance;
struct InstanceCreateInfo;

using UpdateCallback = std::function<void(std::shared_ptr<Engine>)>;
struct EngineCreateInfo {
  UpdateCallback updateCallback;
};

class Engine : public std::enable_shared_from_this<Engine> {
public:
  using Ptr = std::shared_ptr<Engine>;

  Engine() = delete;
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(EngineCreateInfo);
  Engine(Engine&&) = default;
  Engine& operator=(Engine&&) = default;

  std::shared_ptr<Instance> createInstance(InstanceCreateInfo);
  void run();
  void setUpdatesPerSecond(uint32_t count) { updatesPerSecond = count; }

private:
  void initInputCallbacks();
  void renderThreadFunc();
  void handleOSMessages();
  void acquireUpdateSlot();
  void setLastUpdated(int32_t);
  void acquireRenderSlot();
  Clock::duration updateDuration() { return OneSecond / updatesPerSecond; }

  EngineCreateInfo engineCreateInfo;
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

  std::shared_ptr<spdlog::sinks::sink> fileSink;
  std::shared_ptr<spdlog::sinks::sink> stdoutSink;
  std::shared_ptr<spdlog::sinks::sink> stderrSink;
  std::shared_ptr<spdlog::logger> multilogger;
};
}  // namespace vka