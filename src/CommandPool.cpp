#include "CommandPool.hpp"
#include "VulkanFunctionLoader.hpp"
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

void CommandPool::reset() { vkResetCommandPool(device, poolHandle, 0); }

CommandPool::~CommandPool() {
  vkDestroyCommandPool(device, poolHandle, nullptr);
}
}  // namespace vka