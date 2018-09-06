#pragma once

#include "VulkanFunctionLoader.hpp"
#include <glfw/glfw3.h>
#include <memory>
#include <vector>
#include "RenderObject.hpp"

namespace vka {

class Instance;
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
  Surface(std::shared_ptr<Instance>, SurfaceCreateInfo);
  ~Surface();
  std::shared_ptr<Instance> getInstance() { return instance.lock(); }

private:
  std::weak_ptr<Instance> instance;
  VkSurfaceKHR surfaceHandle;
  GLFWwindow* windowHandle;
};
}  // namespace vka