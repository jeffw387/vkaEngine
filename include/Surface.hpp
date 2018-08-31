#pragma once

#include <glfw/glfw3.h>
#include <memory>
#include <vector>
#include "Instance.hpp"
#include "RenderObject.hpp"
#include "VulkanFunctionLoader.hpp"

namespace vka {
class Surface : RenderObject {
public:
  VkSurfaceKHR getHandle() { return surfaceHandle; }
  Surface() = delete;
  Surface(std::shared_ptr<Instance>);

private:
  VkSurfaceKHR surfaceHandle;
  GLFWwindow* window;
  void validateImpl() override;
};
}  // namespace vka