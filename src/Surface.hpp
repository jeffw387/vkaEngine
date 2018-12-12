#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

#include "Input.hpp"

namespace vka {

struct SurfaceCreateInfo {
  int width;
  int height;
  const char* windowTitle;
};

class SurfaceBase {
public:
  operator VkSurfaceKHR() { return surface; }
  SurfaceBase(VkInstance instance, SurfaceCreateInfo createInfo) {
    
  }
  virtual ~SurfaceBase() {}
  [[nodiscard]] virtual bool handleOSMessages() = 0;

  Input::Manager inputManager;

private:
  VkInstance instance;
  VkSurfaceKHR surface;
};

template <typename PlatformT>
class Surface : public SurfaceBase {
public:
  Surface(VkInstance instance, SurfaceCreateInfo createInfo)
   : SurfaceBase(instance, createInfo) {

  }

  bool SurfaceBase::handleOSMessages() {
  }
};
private:
  typename PlatformT::WindowType* window;
};

}  // namespace vka