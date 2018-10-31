#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
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
    if (instanceHandle != VK_NULL_HANDLE && surfaceHandle != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instanceHandle, surfaceHandle, nullptr);
    }
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
  operator VkSurfaceKHR() { return surfaceHandle; }
  operator GLFWwindow*() { return windowHandle; }
  Surface() = delete;
  Surface(VkInstance, SurfaceCreateInfo);
  Surface(Surface&&) = default;
  Surface(const Surface&) = delete;
  Surface& operator=(Surface&&) = default;
  Surface& operator=(const Surface&) = delete;
  ~Surface() = default;

private:
  // VkInstance instance;
  VkSurfaceKHR surfaceHandle;
  SurfaceOwner surfaceOwner;
  GLFWwindow* windowHandle;
  WindowOwner windowOwner;
};
}  // namespace vka