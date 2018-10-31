#include "Surface.hpp"
#include <vulkan/vulkan.h>
#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"
#include "Engine.hpp"

namespace vka {
Surface::Surface(VkInstance instance, SurfaceCreateInfo surfaceCreateInfo) {
  MultiLogger::get()->info("Creating surface.");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  windowHandle = glfwCreateWindow(
      surfaceCreateInfo.width,
      surfaceCreateInfo.height,
      surfaceCreateInfo.windowTitle,
      nullptr,
      nullptr);
  windowOwner = WindowOwner(windowHandle);
  auto surfaceResult =
      glfwCreateWindowSurface(instance, windowHandle, nullptr, &surfaceHandle);
  if (surfaceResult != VK_SUCCESS) {
    MultiLogger::get()->error(
        "Surface not created, result code {}.", surfaceResult);
  }
  surfaceOwner = SurfaceOwner(surfaceHandle, instance);
}
}  // namespace vka