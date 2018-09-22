#include "VulkanFunctionLoader.hpp"
#include "ShaderModule.hpp"
#include "Device.hpp"
#include <vector>

namespace vka {
ShaderModule::ShaderModule(
    VkDevice device,
    const std::vector<uint32_t>& shaderBytes)
    : device(device) {
  VkShaderModuleCreateInfo createInfo{
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = shaderBytes.size() * sizeof(uint32_t);
  createInfo.pCode = shaderBytes.data();
  vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
}

ShaderModule::~ShaderModule() {
  vkDestroyShaderModule(device, shaderModule, nullptr);
}
}  // namespace vka