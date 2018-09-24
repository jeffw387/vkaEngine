#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;

class DescriptorSetLayout {
public:
  DescriptorSetLayout() = delete;
  DescriptorSetLayout(
      VkDevice device,
      const std::vector<VkDescriptorSetLayoutBinding>&);
  DescriptorSetLayout(DescriptorSetLayout&&) = default;
  DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
  ~DescriptorSetLayout();

  operator VkDescriptorSetLayout() { return layoutHandle; }
  const std::vector<VkDescriptorSetLayoutBinding>& getBindings() {
    return bindings;
  }

private:
  VkDevice device;
  VkDescriptorSetLayout layoutHandle;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};
}  // namespace vka