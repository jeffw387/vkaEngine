#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "overloaded.hpp"

namespace vka {
DescriptorSetLayout::DescriptorSetLayout(
    VkDevice device,
    DescriptorBindings bindings)
    : device(device), bindings(std::move(bindings)) {
  VkDescriptorSetLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

  for (auto [binding, descriptors] : this->bindings) {
    if (descriptors.size() > 0) {
      VkDescriptorSetLayoutBinding vkBinding{};
      vkBinding.binding = binding;
      vkBinding.descriptorCount;
      std::visit([&](auto descriptor) {
        vkBinding.descriptorType = static_cast<VkDescriptorType>(
          descriptor.descriptorType());
      }, **descriptors.begin());
      vkBinding.
    }
  }
  createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
  createInfo.pBindings = vkBindings.data();
  vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layoutHandle);
}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (device != VK_NULL_HANDLE && layoutHandle != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device, layoutHandle, nullptr);
  }
}
}  // namespace vka

// Descriptor Set contains bindings
// Bindings contain descriptors of a single type (array of descriptors)
// each binding must specify which shader stages use it