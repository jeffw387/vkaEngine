#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Descriptors.hpp"

namespace vka {
class DescriptorSetLayout {
public:
  DescriptorSetLayout(
      VkDevice device,
      DescriptorBindings);
  ~DescriptorSetLayout();

  operator VkDescriptorSetLayout() { return layoutHandle; }
  DescriptorBindings getBindings() {
    return bindings;
  }

private:
  VkDevice device;
  VkDescriptorSetLayout layoutHandle;
  DescriptorBindings bindings;
  std::vector<VkDescriptorSetLayoutBinding> vkBindings;
};
}  // namespace vka