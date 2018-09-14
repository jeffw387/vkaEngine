#include "Surface.hpp"
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "VulkanFunctionLoader.hpp"
#include "spdlog/spdlog.h"
#include "Engine.hpp"

namespace vka {
Surface::Surface(Instance* instance, SurfaceCreateInfo surfaceCreateInfo)
    : instance(instance) {
  multilogger = spdlog::get(LoggerName);
  multilogger->info("Creating surface.");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  windowHandle = glfwCreateWindow(
      surfaceCreateInfo.width,
      surfaceCreateInfo.height,
      surfaceCreateInfo.windowTitle,
      nullptr,
      nullptr);
  windowOwner = WindowOwner(windowHandle);
  auto surfaceResult = glfwCreateWindowSurface(
      instance->getHandle(), windowHandle, nullptr, &surfaceHandle);
  if (surfaceResult != VK_SUCCESS) {
    multilogger->error("Surface not created, result code {}.", surfaceResult);
  }
  surfaceOwner = SurfaceOwner(surfaceHandle, instance->getHandle());
}
}  // namespace vka