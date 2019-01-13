#pragma once

#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>

namespace vka {
struct pipeline_layout {
  pipeline_layout(VkDevice device, VkPipelineLayout layout)
      : m_device(device), m_layout(layout) {}
  pipeline_layout(const pipeline_layout&) = delete;
  pipeline_layout(pipeline_layout&&) = default;
  pipeline_layout& operator=(const pipeline_layout&) = delete;
  pipeline_layout& operator=(pipeline_layout&&) = default;
  ~pipeline_layout() noexcept {
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
  }
  operator VkPipelineLayout() const noexcept { return m_layout; }

private:
  VkDevice m_device = {};
  VkPipelineLayout m_layout = {};
};

struct pipeline_layout_builder {
  tl::expected<std::unique_ptr<pipeline_layout>, VkResult> build(
      VkDevice device) {
    VkPipelineLayoutCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.pushConstantRangeCount =
        static_cast<uint32_t>(m_pushRanges.size());
    createInfo.pPushConstantRanges = m_pushRanges.data();
    createInfo.setLayoutCount = static_cast<uint32_t>(m_layouts.size());
    createInfo.pSetLayouts = m_layouts.data();

    VkPipelineLayout layout = {};
    auto result = vkCreatePipelineLayout(device, &createInfo, nullptr, &layout);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<pipeline_layout>(device, layout);
  }

  pipeline_layout_builder&
  push_range(VkShaderStageFlags stages, uint32_t offset, uint32_t size) {
    m_pushRanges.push_back({stages, offset, size});
    return *this;
  }

  pipeline_layout_builder& set_layout(VkDescriptorSetLayout layout) {
    m_layouts.push_back(layout);
    return *this;
  }

private:
  std::vector<VkPushConstantRange> m_pushRanges = {};
  std::vector<VkDescriptorSetLayout> m_layouts = {};
};
}  // namespace vka