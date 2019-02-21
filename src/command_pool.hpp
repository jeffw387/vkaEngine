#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>

namespace vka {
struct command_pool {
  explicit command_pool(
      VkDevice device,
      VkCommandPool pool,
      bool canResetBuffers)
      : m_device(device),
        m_pool(pool),
        m_canResetBuffers(canResetBuffers) {}
  command_pool(const command_pool&) = delete;
  command_pool(command_pool&&) = default;
  command_pool& operator=(const command_pool&) = delete;
  command_pool& operator=(command_pool&&) = default;
  ~command_pool() {
    vkDestroyCommandPool(m_device, m_pool, nullptr);
  }
  operator VkCommandPool() { return m_pool; }
  bool can_reset_buffers() const noexcept {
    return m_canResetBuffers;
  }

private:
  VkDevice m_device = {};
  VkCommandPool m_pool = {};
  bool m_canResetBuffers = {};
};

struct command_pool_builder {
  tl::expected<std::unique_ptr<command_pool>, VkResult>
  build(VkDevice device) {
    VkCommandPoolCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    createInfo.queueFamilyIndex = m_queueFamilyIndex;
    createInfo.flags =
        m_canResetBuffers
            ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            : 0;

    VkCommandPool pool = {};
    auto result = vkCreateCommandPool(
        device, &createInfo, nullptr, &pool);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<command_pool>(
        device, pool, m_canResetBuffers);
  }

  command_pool_builder& queue_family_index(uint32_t index) {
    m_queueFamilyIndex = index;
    return *this;
  }

  command_pool_builder& allow_buffer_reset() {
    m_canResetBuffers = true;
    return *this;
  }

private:
  uint32_t m_queueFamilyIndex = {};
  bool m_canResetBuffers = {};
};
}  // namespace vka