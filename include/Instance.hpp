#pragma once

#include "VulkanFunctionLoader.hpp"
#include <memory>
#include <vector>
#include "Device.hpp"
#include "QueueTraits.hpp"
#include "Surface.hpp"
#include "version.hpp"

namespace vka {

class Surface;
class Device;

class Instance : std::enable_shared_from_this<Instance> {
  friend class Device;

public:
  struct CreateInfo {
    const char* appName;
    uint32_t appMajorVersion;
    uint32_t appMinorVersion;
    uint32_t appPatchVersion;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> layers;
  };
  Instance(CreateInfo);

  std::shared_ptr<Device> addDevice(Device::Requirements*);
  std::shared_ptr<Surface> createSurface(Surface::CreateInfo);
  const std::vector<std::shared_ptr<Device>>& getDevices() { return devices; }
  VkInstance getHandle() { return instanceHandle; }

private:
  CreateInfo instanceCreateInfo;
  LibraryHandle vulkanLibrary;
  VkInstance instanceHandle;
  std::shared_ptr<Surface> surface;
  std::vector<VkPhysicalDevice> physicalDevices;
  std::vector<std::shared_ptr<Device>> devices;
};
}  // namespace vka