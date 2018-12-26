#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace vka {
class Device;

class DescriptorSetLayout {
public:
  DescriptorSetLayout(
      VkDevice device,
      std::vector<VkDescriptorSetLayoutBinding>);
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