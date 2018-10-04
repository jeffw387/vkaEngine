#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vka {
class DescriptorSetLayout;

class DescriptorSet {
public:
  DescriptorSet() = default;
  DescriptorSet(VkDescriptorSet set, DescriptorSetLayout* layout);
  DescriptorSet(DescriptorSet&&) = default;
  DescriptorSet(const DescriptorSet&) = default;
  DescriptorSet& operator=(DescriptorSet&&) = default;
  DescriptorSet& operator=(const DescriptorSet&) = default;
  ~DescriptorSet() = default;
  operator VkDescriptorSet();

  VkWriteDescriptorSet createImageDescriptorWrite(
      uint32_t binding,
      uint32_t arrayElement,
      const std::vector<VkDescriptorImageInfo>& imageInfos);

  VkWriteDescriptorSet createBufferDescriptorWrite(
      uint32_t binding,
      uint32_t arrayElement,
      const std::vector<VkDescriptorBufferInfo>& bufferInfos);

private:
  VkDescriptorSet set;
  DescriptorSetLayout* layout;
};

class DescriptorPool {
public:
  DescriptorPool() = default;
  DescriptorPool(
      VkDevice device,
      const std::vector<VkDescriptorPoolSize>& poolSizes,
      uint32_t maxSets);
  DescriptorPool(DescriptorPool&&) = default;
  DescriptorPool& operator=(DescriptorPool&&) = default;
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;
  ~DescriptorPool();
  operator VkDescriptorPool() { return poolHandle; }

  auto allocateDescriptorSets(const std::vector<DescriptorSetLayout*>& layouts);
  void reset();
  void destroy();

private:
  VkDevice device = VK_NULL_HANDLE;
  VkDescriptorPool poolHandle = VK_NULL_HANDLE;
};
}  // namespace vka