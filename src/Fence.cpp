#include "Fence.hpp"
#include <utility>

namespace vka {
Fence::Fence(VkDevice device, bool signaled) : device(device) {
  VkFenceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
  vkCreateFence(device, &createInfo, nullptr, &fence);
}

Fence::Fence(Fence&& other) { *this = std::move(other); }

Fence& Fence::operator=(Fence&& other) {
  if (this != &other) {
    device = other.device;
    fence = other.fence;
    other.device = {};
    other.fence = {};
  }
  return *this;
}

Fence::operator VkFence() { return fence; }

Fence::~Fence() {
  if (device != VK_NULL_HANDLE && fence != VK_NULL_HANDLE) {
    vkDestroyFence(device, fence, nullptr);
  }
}
}  // namespace vka