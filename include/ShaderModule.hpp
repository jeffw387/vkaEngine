#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;

class ShaderModule {
public:
  ShaderModule() = delete;
  ShaderModule(VkDevice device, std::vector<char>& shaderBytes);
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
