#include "DescriptorSetLayout.hpp"
#include "Device.hpp"

namespace vka {
DescriptorSetLayout::DescriptorSetLayout(
    VkDevice device,
    std::vector<VkDescriptorSetLayoutBinding> bindings)
    : device(device), bindings(std::move(bindings)) {
  VkDescriptorSetLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  createInfo.bindingCount = static_cast<uint32_t>(this->bindings.size());
  createInfo.pBindings = this->bindings.data();
  vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layoutHandle);
}

DescriptorSetLayout& DescriptorSetLayout::operator=(
    DescriptorSetLayout&& other) {
  if (this != &other) {
    device = other.device;
    layoutHandle = other.layoutHandle;
    bindings = std::move(other.bindings);
    other.device = {};
    other.layoutHandle = {};
  }
  return *this;
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) {
  *this = std::move(other);
}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (device != VK_NULL_HANDLE && layoutHandle != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device, layoutHandle, nullptr);
  }
}
}  // namespace vka