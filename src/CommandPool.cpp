#include "CommandPool.hpp"
#include "VulkanFunctionLoader.hpp"
#include "Device.hpp"

namespace vka {
CommandPool::CommandPool(Device* device) : device(device) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = device->gfxQueueIndex();

  auto poolResult = vkCreateCommandPool(
      device->getHandle(), &createInfo, nullptr, &poolHandle);
}

auto CommandPool::allocateCommandBuffers(
    size_t count,
    VkCommandBufferLevel level) {
  std::vector<VkCommandBuffer> result;
  result.resize(count);
  VkCommandBufferAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = static_cast<uint32_t>(count);
  allocateInfo.commandPool = poolHandle;
  allocateInfo.level = level;
  vkAllocateCommandBuffers(device->getHandle(), &allocateInfo, result.data());
  return result;
}

void CommandPool::reset() {
  vkResetCommandPool(device->getHandle(), poolHandle, 0);
}

CommandPool::~CommandPool() {
  vkDestroyCommandPool(device->getHandle(), poolHandle, nullptr);
}
}  // namespace vka