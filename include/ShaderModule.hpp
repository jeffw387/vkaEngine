#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace vka {
class Device;

class ShaderModule {
public:
  ShaderModule(VkDevice device, std::vector<char> shaderBytes);
  ShaderModule(ShaderModule&&);
  ShaderModule& operator=(ShaderModule&&);
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ~ShaderModule();

  operator VkShaderModule() { return shaderModule; }

private:
  VkDevice device = VK_NULL_HANDLE;
  VkShaderModule shaderModule = VK_NULL_HANDLE;
};
}  // namespace vka
