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
  SurfaceBase(VkInstance instance, SurfaceCreateInfo createInfo)
      : instance(instance) {}
  virtual ~SurfaceBase(){}[[nodiscard]] virtual bool handleOSMessages() = 0;

  Input::Manager inputManager;
  double mouseX = {};
  double mouseY = {};

protected:
  VkInstance instance;
  VkSurfaceKHR surface;
};

template <typename PlatformT>
class Surface : public SurfaceBase {
public:
  Surface(VkInstance instance, SurfaceCreateInfo createInfo)
      : SurfaceBase(instance, createInfo) {
    window = PlatformT::createWindow(
        createInfo.width, createInfo.height, createInfo.windowTitle);
    surface = PlatformT::createSurface(instance, window);
  }

  bool handleOSMessages() override {}

private:
  typename PlatformT::WindowType* window;
};

}  // namespace vka