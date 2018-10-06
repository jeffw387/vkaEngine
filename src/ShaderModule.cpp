#include <GLFW/glfw3.h>
#include "ShaderModule.hpp"
#include "Device.hpp"
#include <vector>

namespace vka {
ShaderModule::ShaderModule(
    VkDevice device,
    const std::vector<char>& shaderBytes)
    : device(device) {
  VkShaderModuleCreateInfo createInfo{
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = shaderBytes.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
  vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
}

ShaderModule::~ShaderModule() {
  vkDestroyShaderModule(device, shaderModule, nullptr);
}
}  // namespace vka