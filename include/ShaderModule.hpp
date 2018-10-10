#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include <vector>

namespace vka {
class Device;

class ShaderModule {
public:
  ShaderModule() = default;
  ShaderModule(VkDevice device, const std::vector<char>& shaderBytes);
  ShaderModule(ShaderModule&&) = default;
  ShaderModule& operator=(ShaderModule&&) = default;
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ~ShaderModule();

  operator VkShaderModule() { return shaderModule; }

private:
  VkDevice device = VK_NULL_HANDLE;
  VkShaderModule shaderModule = VK_NULL_HANDLE;
};
}  // namespace vka
