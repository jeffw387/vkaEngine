#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vka {
class Device;

class PipelineLayout {
public:
  PipelineLayout(
      VkDevice device,
      std::vector<VkPushConstantRange> pushRanges,
      std::vector<VkDescriptorSetLayout> setLayouts);
  ~PipelineLayout();
  void cmdExecuted() {}

  operator VkPipelineLayout() { return layoutHandle; }

private:
  VkDevice device;
  VkPipelineLayout layoutHandle;
};
}  // namespace vka