#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <fstream>
#include "Engine.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "spdlog/spdlog.h"
#include "Surface.hpp"
// #include "Config.hpp"
#include "Logger.hpp"

namespace vka {
Engine::Engine(EngineCreateInfo engineCreateInfo)
    : updateCallback(engineCreateInfo.updateCallback),
      renderCallback(engineCreateInfo.renderCallback) {
  // sometimes valve's overlay causes problems, this next line will disable it
  // on windows
  // _putenv("DISABLE_VK_LAYER_VALVE_steam_overlay_1=1");

  MultiLogger::get()->info("Logger initialized.");
  MultiLogger::get()->info("Creating Engine.");
  glfwInstance = std::make_unique<GLFW>();
}

std::unique_ptr<Instance> Engine::createInstance(
    InstanceCreateInfo instanceCreateInfo) {
  return std::make_unique<Instance>(this, instanceCreateInfo);
}

void Engine::renderThreadFunc() {
  continueRendering = true;
  while (true) {
    acquireRenderSlot();
    renderCallback();
    if (!continueRendering || !continueUpdating) {
      return;
    }
  }
}

void Engine::run() {
  startTime = Clock::now();
  markStateUpdated(0, startTime);
  MultiLogger::get()->log(spdlog::level::info, "Running engine.");

  MultiLogger::get()->info("Starting render thread.");
  std::thread renderThread(&Engine::renderThreadFunc, this);

  continueUpdating = true;
  MultiLogger::get()->info("Starting update loop.");
  while (true) {
    auto lastUpdateTime =
        (lastUpdatedIndex != -1) ? updateTimes[lastUpdatedIndex] : startTime;
    auto nextUpdateTime = lastUpdateTime + updateDuration();
    auto currentTime = Clock::now();
    if (nextUpdateTime < currentTime) {
      acquireUpdateSlot();
      updateTime = nextUpdateTime;
      if (updateCallback) {
        updateCallback();
        markStateUpdated(updateIndex, updateTime);
      }
    }

    if (!continueRendering || !continueUpdating) {
      break;
    }
  }
  renderThread.join();
}

void Engine::acquireUpdateSlot() {
  std::scoped_lock updateLock(stateMutex);
  for (int32_t i = 0; i < BufferCount; ++i) {
    if (i == renderIndex || i == lastUpdatedIndex) {
      continue;
    }
    updateIndex = i;
    return;
  }
  MultiLogger::get()->error("Error acquiring update slot, no valid indices.");
}

void Engine::markStateUpdated(int32_t index, Clock::time_point updateTime) {
  std::lock_guard<std::mutex> markUpdatedLock{stateMutex};
  lastUpdatedIndex = index;
  updateTimes[index] = updateTime;
}

void Engine::acquireRenderSlot() {
  std::scoped_lock renderLock(stateMutex);
  for (int32_t i = 0; i < BufferCount; ++i) {
    if (i == lastUpdatedIndex) {
      renderIndex = i;
      return;
    }
  }
}
}  // namespace vka