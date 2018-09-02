#include "Engine.hpp"
#include <GLFW/glfw3.h>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "Instance.hpp"

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

Engine::Engine(Engine::CreateInfo engineCreateInfo)
    : engineCreateInfo(engineCreateInfo),
      instance(
          std::make_shared<Instance>(engineCreateInfo.instanceCreateInfo)) {
  auto surface = instance->createSurface(engineCreateInfo.surfaceCreateInfo);
  initInputCallbacks(surface->getWindowHandle());
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

void Engine::initInputCallbacks(GLFWwindow* windowHandle) {
  glfwSetMouseButtonCallback(windowHandle, mouseButtonCallback);
  glfwSetKeyCallback(windowHandle, keyCallback);
  glfwSetCursorPosCallback(windowHandle, cursorPositionCallback);
}

void Engine::handleOSMessages() {
  glfwPollEvents();
  if (glfwWindowShouldClose) {
    continueUpdating = false;
    continueRendering = false;
  }
};

void Engine::run() {
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
  std::runtime_error("Error acquiring update slot, no valid indices.");
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