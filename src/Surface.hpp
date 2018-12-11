#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace vka {

class Engine;
class Instance;

struct SurfaceCreateInfo {
  int width;
  int height;
  const char* windowTitle;
};

template <typename SurfaceSource>
class Surface {
public:
  operator VkSurfaceKHR() { return surface; }
  operator GLFWwindow*() { return window; }
  Surface(Engine* engine, VkInstance instance, SurfaceCreateInfo) {
    
  }
  ~Surface();
  [[nodiscard]] bool handleOSMessages();

private:
  Engine* engine;
  VkInstance instance;
  SurfaceSource::Window* window;
  VkSurfaceKHR surface;
};
}  // namespace vka