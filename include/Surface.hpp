#pragma once

#include "VulkanFunctionLoader.hpp"
#include <glfw/glfw3.h>
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"

namespace vka {

class Instance;

struct SurfaceDeleter {
  using pointer = VkSurfaceKHR;
  VkInstance instanceHandle;

  SurfaceDeleter() = default;
  SurfaceDeleter(VkInstance instanceHandle) : instanceHandle(instanceHandle) {}
  void operator()(VkSurfaceKHR surfaceHandle) {
    vkDestroySurfaceKHR(instanceHandle, surfaceHandle, nullptr);
  }
};
using SurfaceOwner = std::unique_ptr<VkSurfaceKHR, SurfaceDeleter>;

struct WindowDeleter {
  void operator()(GLFWwindow* window) { glfwDestroyWindow(window); }
};
using WindowOwner = std::unique_ptr<GLFWwindow, WindowDeleter>;

struct SurfaceCreateInfo {
  int width;
  int height;
  const char* windowTitle;
};

class Surface {
public:
  VkSurfaceKHR getSurfaceHandle() { return surfaceHandle; }
  GLFWwindow* getWindowHandle() { return windowHandle; }
  Surface() = delete;
  Surface(VkInstance, SurfaceCreateInfo);
  Surface(Surface&&) = default;
  Surface(const Surface&) = delete;
  Surface& operator=(Surface&&) = default;
  Surface& operator=(const Surface&) = delete;
  ~Surface() = default;

private:
  std::shared_ptr<spdlog::logger> multilogger;
  VkInstance instance;
  VkSurfaceKHR surfaceHandle;
  SurfaceOwner surfaceOwner;
  GLFWwindow* windowHandle;
  WindowOwner windowOwner;
};
}  // namespace vka