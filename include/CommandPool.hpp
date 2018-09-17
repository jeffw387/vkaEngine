#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;
class CommandPool {
public:
  CommandPool(VkDevice device, uint32_t gfxQueueIndex);
  CommandPool() = delete;
  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;
  CommandPool(CommandPool&&) = default;
  CommandPool& operator=(CommandPool&&) = default;
  ~CommandPool();

  operator VkCommandPool() { return poolHandle; }
  auto allocateCommandBuffers(size_t count, VkCommandBufferLevel level);
  void reset();

private:
  VkDevice device;
  VkCommandPool poolHandle;
};
}  // namespace vka