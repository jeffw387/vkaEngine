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


}  // namespace vka