#include "Device.hpp"
#include <memory>
#include "Instance.hpp"

namespace vka {
Device::Device(std::shared_ptr<Instance> instance) {}
void Device::validateImpl() {
  VkPhysicalDeviceFeatures enabledFeatures{};
  for (auto feature : requiredFeatures) {
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
  for (auto trait : queueTraits) {
  }
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  // createInfo.
}
}  // namespace vka