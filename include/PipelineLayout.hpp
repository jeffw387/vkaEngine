#pragma once

#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include <vector>

namespace vka {
class Device;

class PipelineLayout {
public:
  PipelineLayout() = default;
  PipelineLayout(
      VkDevice device,
      std::vector<VkPushConstantRange> pushRanges,
      std::vector<VkDescriptorSetLayout> setLayouts);
  PipelineLayout(PipelineLayout&&);
  PipelineLayout& operator=(PipelineLayout&&);
  PipelineLayout(const PipelineLayout&) = delete;
  PipelineLayout& operator=(const PipelineLayout&) = delete;
  ~PipelineLayout();

  operator VkPipelineLayout() { return layoutHandle; }

private:
  VkDevice device;
  VkPipelineLayout layoutHandle;
};
}  // namespace vka