#pragma once

#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;

class PipelineLayout {
public:
  PipelineLayout() = delete;
  PipelineLayout(Device* device, 
    const std::vector<VkPushConstantRange>& pushRanges,
    const std::vector<VkDescriptorSetLayout>& setLayouts);
  PipelineLayout(PipelineLayout&&) = default;
  PipelineLayout& operator=(PipelineLayout&&) = default;
  PipelineLayout(const PipelineLayout&) = delete;
  PipelineLayout& operator=(const PipelineLayout&) = delete;
  ~PipelineLayout();

  VkPipelineLayout getHandle() { return layoutHandle; }
private:
  Device* device;
  VkPipelineLayout layoutHandle;
};
}