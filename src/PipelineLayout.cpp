#include "PipelineLayout.hpp"
#include "Device.hpp"

namespace vka {
PipelineLayout::PipelineLayout(
    Device* device,
    const std::vector<VkPushConstantRange>& pushRanges,
    const std::vector<VkDescriptorSetLayout>& setLayouts) {
  VkPipelineLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
  createInfo.pPushConstantRanges = pushRanges.data();
  createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  createInfo.pSetLayouts = setLayouts.data();
  vkCreatePipelineLayout(
      device->getHandle(), &createInfo, nullptr, &layoutHandle);
}

PipelineLayout::~PipelineLayout() {
  vkDestroyPipelineLayout(device->getHandle(), layoutHandle, nullptr);
}
}  // namespace vka