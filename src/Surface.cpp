#include <vulkan/vulkan.h>
#include "Logger.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Engine.hpp"

namespace vka {
static void
keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
  enginePtr->inputManager.addInputToQueue(
      Input::Event<Input::Key>{{key, action}, Clock::now()});
}

static void
cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  auto enginePtr = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
  enginePtr->mouseX = xpos;
  enginePtr->mouseY = ypos;
}

static void
mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  auto enginePtr = reinterpret_cast<Surface*>(glfwGetWindowUserPointer(window));
  enginePtr->inputManager.addInputToQueue(
      Input::Event<Input::Mouse>{{button, action}, Clock::now()});
}



template <typename SurfaceSource>
Surface<SurfaceSource>::Surface(
    Engine* engine,
    VkInstance instance,
    SurfaceCreateInfo surfaceCreateInfo)
    : engine(engine), instance(instance) {
  MultiLogger::get()->info("Creating surface.");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(
      surfaceCreateInfo.width,
      surfaceCreateInfo.height,
      surfaceCreateInfo.windowTitle,
      nullptr,
      nullptr);
  auto surfaceResult =
      glfwCreateWindowSurface(instance, window, nullptr, &surface);
  if (surfaceResult != VK_SUCCESS) {
    MultiLogger::get()->error(
        "Surface not created, result code {}.", surfaceResult);
  }

  glfwSetWindowUserPointer(window, engine);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, cursorPositionCallback);
}

Surface::~Surface() {
  if (instance != VK_NULL_HANDLE && surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
  }
  glfwDestroyWindow(window);
}
}  // namespace vka