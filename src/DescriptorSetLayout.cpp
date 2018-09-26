#include "DescriptorSetLayout.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vka {
DescriptorSetLayout::DescriptorSetLayout(
    VkDevice device,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    : device(device), bindings(bindings) {
  VkDescriptorSetLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  createInfo.pBindings = bindings.data();
  vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layoutHandle);
}

DescriptorSetLayout::~DescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(device, layoutHandle, nullptr);
}
}  // namespace vka