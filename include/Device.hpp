#pragma once

#include "VulkanFunctionLoader.hpp"
#include "vk_mem_alloc.h"
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"

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

struct DeviceRequirements {
  std::vector<PhysicalDeviceFeatures> requiredFeatures;
  std::vector<const char*> deviceExtensions;
};
class Device : public std::enable_shared_from_this<Device> {
  static constexpr uint32_t U32Max = ~(0ui32);

public:
  Device() = delete;
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(std::shared_ptr<Instance>, DeviceRequirements);
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;
  ~Device();
  VkDevice getHandle() { return deviceHandle; }
  uint32_t gfxQueueIndex() { return graphicsQueueIndex; }
  VmaAllocator getAllocator() { return allocator; }
  std::shared_ptr<Instance> getInstance() { return instance.lock(); }

private:
  std::weak_ptr<Instance> instance;
  DeviceRequirements requirements;
  std::shared_ptr<spdlog::logger> multilogger;
  VkPhysicalDevice physicalDeviceHandle;
  VkPhysicalDeviceProperties deviceProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  VmaAllocator allocator;

  VkDevice deviceHandle;
  uint32_t graphicsQueueIndex = U32Max;
  VkDeviceQueueCreateInfo queueCreateInfo = {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  VkQueue graphicsQueue;

  VkDebugUtilsMessengerEXT debugMessenger;
};

}  // namespace vka