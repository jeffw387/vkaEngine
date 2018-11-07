#include "Fence.hpp"
#include <utility>

namespace vka {
Fence::Fence(VkDevice device, bool signaled) : device(device) {
  VkFenceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
  vkCreateFence(device, &createInfo, nullptr, &fence);
}

void Fence::wait(uint64_t timeout) {
  auto waitResult = vkWaitForFences(device, 1, &fence, true, timeout);
  if (waitResult == VK_SUCCESS) {
    for (auto& sub : subscribers) {
      if (sub) {
        sub();
      }
    }
    subscribers.clear();
  }
}

void Fence::reset() { vkResetFences(device, 1, &fence); }

Fence::operator VkFence() { return fence; }

Fence::~Fence() {
  if (device != VK_NULL_HANDLE && fence != VK_NULL_HANDLE) {
    vkDestroyFence(device, fence, nullptr);
  }
}
}  // namespace vka