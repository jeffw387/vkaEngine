#include "DescriptorPool.hpp"
#include "VulkanFunctionLoader.hpp"
#include "Device.hpp"
#include <vector>

namespace vka {
DescriptorPool::DescriptorPool(
    Device* device,
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets)
    : device(device) {
  VkDescriptorPoolCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();
  createInfo.maxSets = maxSets;
  vkCreateDescriptorPool(
      device->getHandle(), &createInfo, nullptr, &poolHandle);
}

auto DescriptorPool::allocateDescriptorSets(
    std::vector<VkDescriptorSetLayout> layouts) {
  std::vector<VkDescriptorSet> result;
  result.resize(layouts.size());
  VkDescriptorSetAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool = poolHandle;
  allocateInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocateInfo.pSetLayouts = layouts.data();
  vkAllocateDescriptorSets(device->getHandle(), &allocateInfo, result.data());
  return result;
}

void DescriptorPool::reset() {
  vkResetDescriptorPool(device->getHandle(), poolHandle, 0);
}

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(device->getHandle(), poolHandle, nullptr);
}
}  // namespace vka