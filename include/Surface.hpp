#pragma once

#include "VulkanFunctionLoader.hpp"
#include <glfw/glfw3.h>
#include <memory>
#include <vector>
#include "RenderObject.hpp"

namespace vka {

class Instance;

class Surface {
public:
  struct CreateInfo {
    int width;
    int height;
    const char* windowTitle;
  };
  VkSurfaceKHR getSurfaceHandle() { return surfaceHandle; }
  GLFWwindow* getWindowHandle() { return windowHandle; }
  Surface() = delete;
  Surface(std::shared_ptr<Instance>, CreateInfo);
  ~Surface();

private:
  std::shared_ptr<Instance> instance;
  VkSurfaceKHR surfaceHandle;
  GLFWwindow* windowHandle;
};
}  // namespace vka