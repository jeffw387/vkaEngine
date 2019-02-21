#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>

namespace vka {
struct fence {
  explicit fence(VkDevice device, VkFence fence)
      : m_device(device), m_fence(fence) {}

  fence(const fence&) = delete;
  fence(fence&&) = default;
  fence& operator=(const fence&) = delete;
  fence& operator=(fence&&) = default;

  ~fence() noexcept {
    vkDestroyFence(m_device, m_fence, nullptr);
  }

  operator VkFence() const noexcept { return m_fence; }

private:
  VkDevice m_device = {};
  VkFence m_fence = {};
};

struct fence_builder {
  tl::expected<std::unique_ptr<fence>, VkResult> build(
      VkDevice device) {
    VkFenceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    createInfo.flags |=
        (m_createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT
                          : 0);

    VkFence fenceHandle = {};
    auto result = vkCreateFence(
        device, &createInfo, nullptr, &fenceHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<fence>(device, fenceHandle);
  }

  fence_builder& signaled() {
    m_createSignaled = true;
    return *this;
  }

private:
  bool m_createSignaled = {};
};
}  // namespace vka