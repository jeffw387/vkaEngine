#include "Engine.hpp"
#include <GLFW/glfw3.h>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "Instance.hpp"
#include <memory>
#include "spdlog/spdlog.h"
#include "Surface.hpp"

namespace vka {

static void keyCallback(
  GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void cursorPositionCallback(
  GLFWwindow* window, double xpos, double ypos) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

static void mouseButtonCallback(
  GLFWwindow* window, int button, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
}

Engine::Engine(EngineCreateInfo engineCreateInfo)
  : engineCreateInfo(engineCreateInfo),
    updateCallback(engineCreateInfo.updateCallback) {
  fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFileName);
  fileSink->set_level(spdlog::level::trace);
  stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  stdoutSink->set_level(spdlog::level::info);
  stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  stderrSink->set_level(spdlog::level::err);
  std::vector<spdlog::sink_ptr> sinks = {fileSink, stdoutSink, stderrSink};
  multilogger =
    std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
}

std::shared_ptr<Instance> Engine::createInstance(
  InstanceCreateInfo instanceCreateInfo) {
  instance = std::make_shared<Instance>(shared_from_this(), instanceCreateInfo);
  return instance;
}

void Engine::renderThreadFunc() {
  continueRendering = true;
  while (true) {
    acquireRenderSlot();

    if (!continueRendering || !continueUpdating) {
      return;
    }
  }
}

void Engine::initInputCallbacks() {
  auto windowHandle = instance->getSurface()->getWindowHandle();
  glfwSetMouseButtonCallback(windowHandle, mouseButtonCallback);
  glfwSetKeyCallback(windowHandle, keyCallback);
  glfwSetCursorPosCallback(windowHandle, cursorPositionCallback);
}

void Engine::handleOSMessages() {
  auto windowHandle = instance->getSurface()->getWindowHandle();
  glfwPollEvents();
  if (glfwWindowShouldClose(windowHandle)) {
    continueUpdating = false;
    continueRendering = false;
  }
};

void Engine::run() {
  multilogger->log(spdlog::level::info, "Running engine.");
  if (!instance) {
    multilogger->critical("No instance found!");
  }
  if (!(instance->getSurface())) {
    multilogger->warn("No surface found!");
  }
  if (!(instance->getDevice())) {
    multilogger->critical("No device found!");
  }

  multilogger->info("Creating input callbacks.");
  initInputCallbacks();

  std::thread renderThread(&Engine::renderThreadFunc, this);

  continueUpdating = true;
  while (true) {
    auto nextUpdateTime = indexUpdateTime[lastUpdatedIndex] + updateDuration();
    auto currentTime = Clock::now();
    if (nextUpdateTime < currentTime) {
      acquireUpdateSlot();
      if (updateCallback) {
        updateCallback(shared_from_this());
      }
      setLastUpdated(updateIndex);
    }

    handleOSMessages();

    if (!continueRendering || !continueUpdating) {
      break;
    }
  }
  renderThread.join();
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
  }
  multilogger->error("Error acquiring update slot, no valid indices.");
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