#include "PipelineLayout.hpp"
#include "Device.hpp"

namespace vka {
PipelineLayout::PipelineLayout(
    VkDevice device,
    const std::vector<VkPushConstantRange>& pushRanges,
    const std::vector<VkDescriptorSetLayout>& setLayouts)
    : device(device) {
  VkPipelineLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
  createInfo.pPushConstantRanges = pushRanges.data();
  createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  createInfo.pSetLayouts = setLayouts.data();
  vkCreatePipelineLayout(device, &createInfo, nullptr, &layoutHandle);
}

PipelineLayout::~PipelineLayout() {
  vkDestroyPipelineLayout(device, layoutHandle, nullptr);
}
}  // namespace vka