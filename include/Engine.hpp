#pragma once
#include <GLFW/glfw3.h>
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
#include "Asset.hpp"

namespace vka {
static constexpr auto BufferCount = 3U;
using Clock = std::chrono::high_resolution_clock;
static constexpr Clock::duration OneSecond = std::chrono::seconds(1);
static auto LogFileName = "vkaEngineLog.txt";
static auto LoggerName = "MultiLogger";

class Engine;
class Instance;
struct InstanceCreateInfo;

using UpdateCallback = std::function<void(Engine*)>;
using RenderCallback = std::function<void(Engine*)>;
struct EngineCreateInfo {
  UpdateCallback updateCallback;
  RenderCallback renderCallback;
};

class Engine {
public:
  Engine() = default;
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(EngineCreateInfo);
  Engine(Engine&&) = default;
  Engine& operator=(Engine&&) = default;
  ~Engine() = default;

  Instance* createInstance(InstanceCreateInfo);
  void run();
  void setUpdatesPerSecond(uint32_t count) { updatesPerSecond = count; }
  size_t LoadAsset(const std::string& assetPath);
  int32_t currentUpdateIndex() { return updateIndex; }
  int32_t currentRenderIndex() { return renderIndex; }

private:
  void initInputCallbacks();
  void renderThreadFunc();
  void handleOSMessages();
  void acquireUpdateSlot();
  void setLastUpdated(int32_t);
  void acquireRenderSlot();
  Clock::duration updateDuration() { return OneSecond / updatesPerSecond; }

  std::unique_ptr<Instance> instance;
  std::mutex stateMutex;
  bool running = false;
  int32_t updateIndex = -1;
  int32_t lastUpdatedIndex = -1;
  int32_t renderIndex = -1;
  bool continueRendering;
  bool continueUpdating;
  uint32_t updatesPerSecond;
  Clock::time_point startTime;
  std::array<Clock::time_point, BufferCount> indexUpdateTime;
  UpdateCallback updateCallback;
  RenderCallback renderCallback;

  std::shared_ptr<spdlog::sinks::sink> fileSink;
  std::shared_ptr<spdlog::sinks::sink> stdoutSink;
  std::shared_ptr<spdlog::sinks::sink> stderrSink;
  std::shared_ptr<spdlog::logger> multilogger;
};
}  // namespace vka