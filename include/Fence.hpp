#pragma once
#include <vulkan/vulkan.h>

namespace vka {
class Fence {
public:
  Fence() = default;
  Fence(VkDevice device, bool signaled = true);
  Fence(const Fence&) = delete;
  Fence& operator=(const Fence&) = delete;
  Fence(Fence&&);
  Fence& operator=(Fence&&);
  ~Fence();

  void wait(uint64_t timeout = ~(0ULL));
  void reset();
  operator VkFence();
private:
  VkDevice device = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
};
}