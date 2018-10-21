#include "CommandPool.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vka {
CommandPool::CommandPool(VkDevice device, uint32_t gfxQueueIndex)
    : device(device) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = gfxQueueIndex;

  auto poolResult =
      vkCreateCommandPool(device, &createInfo, nullptr, &poolHandle);
}

std::vector<CommandBuffer> CommandPool::allocateCommandBuffers(
    size_t count,
    VkCommandBufferLevel level) {
  std::vector<CommandBuffer> result;
  result.resize(count);
  VkCommandBufferAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = static_cast<uint32_t>(count);
  allocateInfo.commandPool = poolHandle;
  allocateInfo.level = level;
  vkAllocateCommandBuffers(
      device, &allocateInfo, reinterpret_cast<VkCommandBuffer*>(result.data()));
  return result;
}

CommandPool& CommandPool::operator=(CommandPool&& other) {
  if (this != &other) {
    device = other.device;
    poolHandle = other.poolHandle;
    other.device = {};
    other.poolHandle = {};
  }
  return *this;
}

CommandPool::CommandPool(CommandPool&& other) { *this = std::move(other); }

void CommandPool::reset(bool releaseResources) {
  vkResetCommandPool(
      device,
      poolHandle,
      releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
}

CommandPool::~CommandPool() {
  if (device != VK_NULL_HANDLE && poolHandle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, poolHandle, nullptr);
  }
}
}  // namespace vka