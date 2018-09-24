#include "DescriptorPool.hpp"
#include "VulkanFunctionLoader.hpp"
#include "Device.hpp"
#include <vector>

namespace vka {
DescriptorSet::DescriptorSet(VkDescriptorSet set, DescriptorSetLayout* layout)
    : set(set), layout(layout) {}

DescriptorSet::operator VkDescriptorSet() { return set; }

VkWriteDescriptorSet DescriptorSet::createImageDescriptorWrite(
    uint32_t binding,
    uint32_t arrayElement,
    const std::vector<VkDescriptorImageInfo>& imageInfos) {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.dstArrayElement = arrayElement;
  write.descriptorType = layout->getBindings()[binding].descriptorType;
  write.descriptorCount = static_cast<uint32_t>(imageInfos.size());
  write.pImageInfo = imageInfos.data();
  return write;
}

VkWriteDescriptorSet DescriptorSet::createBufferDescriptorWrite(
    uint32_t binding,
    uint32_t arrayElement,
    const std::vector<VkDescriptorBufferInfo>& bufferInfos) {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.dstArrayElement = arrayElement;
  write.descriptorType = layout->getBindings()[binding].descriptorType;
  write.descriptorCount = static_cast<uint32_t>(bufferInfos.size());
  write.pBufferInfo = bufferInfos.data();
  return write;
}

DescriptorPool::DescriptorPool(
    VkDevice device,
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets)
    : device(device) {
  VkDescriptorPoolCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();
  createInfo.maxSets = maxSets;
  vkCreateDescriptorPool(device, &createInfo, nullptr, &poolHandle);
}

auto DescriptorPool::allocateDescriptorSets(
    const std::vector<DescriptorSetLayout*>& layouts) {
  std::vector<VkDescriptorSetLayout> vkLayouts;
  std::vector<VkDescriptorSet> vkSets;
  std::vector<DescriptorSet> sets;

  vkSets.resize(layouts.size());
  sets.resize(layouts.size());
  for (auto i = 0U; i < layouts.size(); ++i) {
    vkLayouts.push_back(*layouts[i]);
  }
  VkDescriptorSetAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool = poolHandle;
  allocateInfo.descriptorSetCount = static_cast<uint32_t>(vkLayouts.size());
  allocateInfo.pSetLayouts = vkLayouts.data();
  vkAllocateDescriptorSets(device, &allocateInfo, vkSets.data());
  return sets;
}

void DescriptorPool::reset() { vkResetDescriptorPool(device, poolHandle, 0); }

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(device, poolHandle, nullptr);
}
}  // namespace vka