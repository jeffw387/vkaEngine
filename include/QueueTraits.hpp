#pragma once
#include <vulkan/vulkan.h>

namespace vka {
struct QueueTraits {
  QueueTraits(VkQueueFlags mustHave, VkQueueFlags mustNotHave, float priority);

  VkQueueFlags mustHave = 0;
  VkQueueFlags mustNotHave = 0;
  float priority;
};
}  // namespace vka