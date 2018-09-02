#include "Surface.hpp"
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "VulkanFunctionLoader.hpp"

namespace vka {
Surface::Surface(
    std::shared_ptr<Instance> instance, Surface::CreateInfo surfaceCreateInfo)
    : instance(instance) {
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NO_API);
  windowHandle = glfwCreateWindow(
      surfaceCreateInfo.width,
      surfaceCreateInfo.height,
      surfaceCreateInfo.windowTitle,
      nullptr,
      nullptr);
  glfwCreateWindowSurface(
      instance->getHandle(), windowHandle, nullptr, &surfaceHandle);
}

Surface::~Surface() { glfwDestroyWindow(windowHandle); }
}  // namespace vka