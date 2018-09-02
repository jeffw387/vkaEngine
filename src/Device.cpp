#include "Device.hpp"
#include <memory>
#include "Instance.hpp"

namespace vka {
Device::Device(
    std::shared_ptr<Instance> instance, Device::Requirements requirements)
    : requirements(requirements) {
  physicalDeviceHandle = instance->physicalDevices.at(0);
  vkGetPhysicalDeviceProperties(physicalDeviceHandle, &deviceProperties);

  uint32_t queueFamilyPropertyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDeviceHandle, &queueFamilyPropertyCount, nullptr);
  queueFamilyproperties.resize(queueFamilyPropertyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDeviceHandle,
      &queueFamilyPropertyCount,
      queueFamilyproperties.data());

  vkGetPhysicalDeviceMemoryProperties(physicalDeviceHandle, &memoryProperties);

  VkPhysicalDeviceFeatures enabledFeatures{};
  for (auto feature : requirements.requiredFeatures) {
    switch (feature) {
      case PhysicalDeviceFeatures::robustBufferAccess:
        enabledFeatures.robustBufferAccess = true;
        break;
      case PhysicalDeviceFeatures::geometryShader:
        enabledFeatures.geometryShader = true;
        break;
      case PhysicalDeviceFeatures::multiDrawIndirect:
        enabledFeatures.multiDrawIndirect = true;
        break;
      case PhysicalDeviceFeatures::drawIndirectFirstInstance:
        enabledFeatures.drawIndirectFirstInstance = true;
        break;
      case PhysicalDeviceFeatures::fillModeNonSolid:
        enabledFeatures.fillModeNonSolid = true;
        break;
      case PhysicalDeviceFeatures::multiViewport:
        enabledFeatures.multiViewport = true;
        break;
      case PhysicalDeviceFeatures::samplerAnistropy:
        enabledFeatures.samplerAnisotropy = true;
        break;
    }
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  for (auto trait : requirements.queueTraits) {
    auto queueCreateInfo = queueCreateInfos.emplace_back();
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    // queueCreateInfo.
  }
  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(requirements.deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames =
      requirements.deviceExtensions.data();
  vkCreateDevice(
      physicalDeviceHandle, &deviceCreateInfo, nullptr, &deviceHandle);
}
}  // namespace vka