#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

#include "Input.hpp"

namespace vka {

struct SurfaceCreateInfo {
  int width;
  int height;
  const char* windowTitle;
};

class SurfaceBase {
public:
  operator VkSurfaceKHR() { return surface; }
  SurfaceBase(VkInstance instance, SurfaceCreateInfo createInfo)
      : instance(instance) {}
  virtual ~SurfaceBase(){}[[nodiscard]] virtual bool handleOSMessages() = 0;

  Input::Manager inputManager;
  double mouseX = {};
  double mouseY = {};

protected:
  VkInstance instance;
  VkSurfaceKHR surface;
};

static void
keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto surfacePtr =
      reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->inputManager.addInputToQueue(
      Input::Event<Input::Key>{{key, action}, Clock::now()});
}

static void
cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  auto surfacePtr =
      reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->mouseX = xpos;
  surfacePtr->mouseY = ypos;
}

static void
mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  auto surfacePtr =
      reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->inputManager.addInputToQueue(
      Input::Event<Input::Mouse>{{button, action}, Clock::now()});
}

template <typename PlatformT>
class Surface : public SurfaceBase {
public:
  Surface(VkInstance instance, SurfaceCreateInfo createInfo)
      : SurfaceBase(instance, createInfo) {
    window = PlatformT::createWindow(
        createInfo.width, createInfo.height, createInfo.windowTitle);
    surface = PlatformT::createSurface(instance, window);

    PlatformT::setKeyCallback(keyCallback);
    PlatformT::setMouseButtonCallback(mouseButtonCallback);
    PlatformT::setCursorCallback(cursorPositionCallback);
  }

  bool handleOSMessages() override {}

private:
  typename PlatformT::WindowType* window;
};

}  // namespace vka