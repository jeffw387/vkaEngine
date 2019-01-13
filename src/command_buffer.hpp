#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include "command_pool.hpp"

namespace vka {
struct command_buffer {
  explicit command_buffer(
      VkDevice device,
      command_pool* pool,
      VkCommandBuffer commandBuffer,
      bool canReset)
      : m_device(device),
        m_pool(pool),
        m_commandBuffer(commandBuffer),
        m_canReset(canReset) {}
  command_buffer(const command_buffer&) = delete;
  command_buffer(command_buffer&) = default;
  command_buffer& operator=(const command_buffer&) = delete;
  command_buffer& operator=(command_buffer&&) = default;
  ~command_buffer() {
    vkFreeCommandBuffers(m_device, *m_pool, 1, &m_commandBuffer);
  }
  operator VkCommandBuffer() const noexcept { return m_commandBuffer; }
  bool can_reset() const noexcept { return m_canReset; }

private:
  VkDevice m_device = {};
  command_pool* m_pool = {};
  VkCommandBuffer m_commandBuffer = {};
  bool m_canReset = {};
};

struct command_buffer_allocator {
  tl::expected<std::unique_ptr<command_buffer>, VkResult> allocate(
      VkDevice device) {
    VkCommandBufferAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.level = m_level;
    allocateInfo.commandPool = *m_pool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = {};
    auto result =
        vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<command_buffer>(
        device, m_pool, commandBuffer, m_pool->can_reset_buffers());
  }

  command_buffer_allocator& set_command_pool(command_pool* pool) {
    m_pool = pool;
    return *this;
  }

  command_buffer_allocator& level(VkCommandBufferLevel bufferLevel) {
    m_level = bufferLevel;
    return *this;
  }

private:
  command_pool* m_pool = {};
  VkCommandBufferLevel m_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
};
}  // namespace vka
