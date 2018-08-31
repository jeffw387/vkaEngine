#pragma once

#include <memory>
#include <vector>
#include "Instance.hpp"
#include "QueueTraits.hpp"
#include "RenderObject.hpp"
#include "VulkanFunctionLoader.hpp"

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

class Device : RenderObject {
public:
  std::vector<PhysicalDeviceFeatures> requiredFeatures;
  std::vector<const char*> deviceExtensions;
  std::vector<QueueTraits> queueTraits;
  VkDevice getHandle() { return deviceHandle; }
  Device() = delete;
  Device(std::shared_ptr<Instance>);

private:
  VkPhysicalDevice physicalDeviceHandle;
  VkDevice deviceHandle;

  void validateImpl() override;
};

}  // namespace vka