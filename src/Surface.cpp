#include <vulkan/vulkan.h>
#include "Logger.hpp"
#include "Surface.hpp"
#include "Instance.hpp"

namespace vka {
static void
keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto surfacePtr = reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->inputManager.addInputToQueue(
      Input::Event<Input::Key>{{key, action}, Clock::now()});
}

static void
cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  auto surfacePtr = reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->mouseX = xpos;
  surfacePtr->mouseY = ypos;
}

static void
mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  auto surfacePtr = reinterpret_cast<SurfaceBase*>(glfwGetWindowUserPointer(window));
  surfacePtr->inputManager.addInputToQueue(
      Input::Event<Input::Mouse>{{button, action}, Clock::now()});
}
}  // namespace vka