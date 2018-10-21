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
#include "Device.hpp"
#include "spdlog/spdlog.h"
#include "Surface.hpp"
#include "Config.hpp"
#include "Logger.hpp"

namespace vka {

static void
keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void
cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void
mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

Engine::Engine(EngineCreateInfo engineCreateInfo)
    : initCallback(engineCreateInfo.initCallback),
      updateCallback(engineCreateInfo.updateCallback),
      renderCallback(engineCreateInfo.renderCallback) {
  // sometimes valve's overlay causes problems, this next line will disable it
  // on windows
  // _putenv("DISABLE_VK_LAYER_VALVE_steam_overlay_1=1");

  MultiLogger::get()->info("Logger initialized.");
  MultiLogger::get()->info("Creating Engine.");
}

Instance* Engine::createInstance(InstanceCreateInfo instanceCreateInfo) {
  instance = std::make_unique<Instance>(this, instanceCreateInfo);
  return instance.get();
}

void Engine::renderThreadFunc() {
  continueRendering = true;
  while (true) {
    acquireRenderSlot();
    renderCallback(this);
    if (!continueRendering || !continueUpdating) {
      return;
    }
  }
}

void Engine::initInputCallbacks() {
  auto surface = instance->getSurface();
  glfwSetMouseButtonCallback(*surface, mouseButtonCallback);
  glfwSetKeyCallback(*surface, keyCallback);
  glfwSetCursorPosCallback(*surface, cursorPositionCallback);
}

void Engine::handleOSMessages() {
  glfwPollEvents();
  if (glfwWindowShouldClose(*instance->getSurface())) {
    continueUpdating = false;
    continueRendering = false;
  }
};

void Engine::run() {
  running = true;
  initCallback(this, 0);
  lastUpdatedIndex = 0;
  MultiLogger::get()->log(spdlog::level::info, "Running engine.");
  if (!instance) {
    MultiLogger::get()->critical("No instance found!");
  }
  if (!(instance->getSurface())) {
    MultiLogger::get()->warn("No surface found!");
  }
  if (!(instance->getDevice())) {
    MultiLogger::get()->critical("No device found!");
  }

  MultiLogger::get()->info("Creating input callbacks.");
  initInputCallbacks();

  startTime = Clock::now();
  MultiLogger::get()->info("Starting render thread.");
  std::thread renderThread(&Engine::renderThreadFunc, this);

  continueUpdating = true;
  MultiLogger::get()->info("Starting update loop.");
  while (true) {
    auto lastUpdateTime = (lastUpdatedIndex != -1)
                              ? indexUpdateTime[lastUpdatedIndex]
                              : startTime;
    auto nextUpdateTime = lastUpdateTime + updateDuration();
    auto currentTime = Clock::now();
    if (nextUpdateTime < currentTime) {
      acquireUpdateSlot();
      if (updateCallback) {
        updateCallback(this);
      }
      setLastUpdated(updateIndex);
    }

    handleOSMessages();

    if (!continueRendering || !continueUpdating) {
      break;
    }
  }
  renderThread.join();
  vkDeviceWaitIdle(*instance->getDevice());
}

void Engine::setLastUpdated(int32_t index) {
  std::scoped_lock updateFinishLock(stateMutex);
  lastUpdatedIndex = index;
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