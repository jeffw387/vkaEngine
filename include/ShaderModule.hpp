#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;

class ShaderModule {
public:
  ShaderModule() = delete;
  ShaderModule(Device* device, std::vector<char>& shaderBytes);
  ShaderModule(ShaderModule&&) = default;
  ShaderModule& operator=(ShaderModule&&) = default;
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ~ShaderModule();

  VkShaderModule getHandle() { return shaderModule; }

private:
  Device* device;
  VkShaderModule shaderModule;
};
}  // namespace vka
