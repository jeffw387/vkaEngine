#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;
class CommandPool {
public:
  CommandPool(Device* device);
  CommandPool() = delete;
  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;
  CommandPool(CommandPool&&) = default;
  CommandPool& operator=(CommandPool&&) = default;
  ~CommandPool();

  VkCommandPool getHandle() { return poolHandle; }
  auto allocateCommandBuffers(size_t count, VkCommandBufferLevel level);
  void reset();

private:
  Device* device;
  VkCommandPool poolHandle;
};
}  // namespace vka