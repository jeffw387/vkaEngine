#include <GLFW/glfw3.h>
#include "ShaderModule.hpp"
#include "Device.hpp"
#include <vector>

namespace vka {
ShaderModule::ShaderModule(ShaderModule&& other) { *this = std::move(other); }
ShaderModule& ShaderModule::operator=(ShaderModule&& other) {
  if (this != &other) {
    std::swap(device, other.device);
    std::swap(shaderModule, other.shaderModule);
  }
  return *this;
}
ShaderModule::ShaderModule(VkDevice device, std::vector<char> shaderBytes)
    : device(device) {
  VkShaderModuleCreateInfo createInfo{
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = shaderBytes.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
  vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
}

ShaderModule::~ShaderModule() {
  if (device != VK_NULL_HANDLE && shaderModule != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device, shaderModule, nullptr);
  }
}
}  // namespace vka