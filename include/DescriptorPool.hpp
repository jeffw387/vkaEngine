#pragma once
#include <vector>
#include "VulkanFunctionLoader.hpp"

namespace vka {
class Device;

class DescriptorPool {
public:
  DescriptorPool() = delete;
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

  auto allocateDescriptorSets(std::vector<VkDescriptorSetLayout> layouts);
  void reset();

private:
  VkDevice device;
  VkDescriptorPool poolHandle;
};
}  // namespace vka