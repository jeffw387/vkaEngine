#include "Surface.hpp"
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "VulkanFunctionLoader.hpp"
#include "spdlog/spdlog.h"
#include "Engine.hpp"

namespace vka {
Surface::Surface(
  std::shared_ptr<Instance> instance, SurfaceCreateInfo surfaceCreateInfo)
  : instance(instance) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  windowHandle = glfwCreateWindow(
    surfaceCreateInfo.width,
    surfaceCreateInfo.height,
    surfaceCreateInfo.windowTitle,
    nullptr,
    nullptr);
  auto surfaceResult = glfwCreateWindowSurface(
    getInstance()->getHandle(), windowHandle, nullptr, &surfaceHandle);
  if (surfaceResult != VK_SUCCESS) {
    auto multilogger = spdlog::get(LoggerName);
    multilogger->error("Surface not created, result code {}.", surfaceResult);
  }
}

Surface::~Surface() { glfwDestroyWindow(windowHandle); }
}  // namespace vka