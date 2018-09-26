#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include <vector>

namespace vka {
class Device;

class ShaderModule {
public:
  ShaderModule() = delete;
  ShaderModule(VkDevice device, const std::vector<uint32_t>& shaderBytes);
  ShaderModule(ShaderModule&&) = default;
  ShaderModule& operator=(ShaderModule&&) = default;
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ~ShaderModule();

  operator VkShaderModule() { return shaderModule; }

private:
  VkDevice device;
  VkShaderModule shaderModule;
};
}  // namespace vka
