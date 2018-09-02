#pragma once

#include "VulkanFunctionLoader.hpp"
#include <memory>
#include <vector>
#include "QueueTraits.hpp"

namespace vka {
enum class PhysicalDeviceFeatures {
  robustBufferAccess,
  geometryShader,
  multiDrawIndirect,
  drawIndirectFirstInstance,
  fillModeNonSolid,
  multiViewport,
  samplerAnistropy
};

class Instance;

class Device {
public:
  struct Requirements {
    std::vector<PhysicalDeviceFeatures> requiredFeatures;
    std::vector<const char*> deviceExtensions;
    std::vector<QueueTraits> queueTraits;
  };

  VkDevice getHandle() { return deviceHandle; }
  Device() = delete;
  Device(std::shared_ptr<Instance>, Requirements);

private:
  Requirements requirements;
  VkPhysicalDevice physicalDeviceHandle;
  VkPhysicalDeviceProperties deviceProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyproperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  VkDevice deviceHandle;
  std::vector<VkQueue> queues;
};

}  // namespace vka