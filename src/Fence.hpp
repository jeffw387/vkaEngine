#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <vector>

namespace vka {
using FenceSubscriber = std::function<void()>;

class Fence {
public:
  Fence(VkDevice device, bool signaled = true);
  ~Fence();

  template <typename T>
  void subscribe(T subscriber) {
    subscribers.push_back(std::move(subscriber));
  }
  void wait(uint64_t timeout = ~(0ULL));
  void reset();
  operator VkFence();

private:
  VkDevice device = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
  std::vector<FenceSubscriber> subscribers;
};
}  // namespace vka