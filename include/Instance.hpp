#pragma once

#include <memory>
#include <vector>
#include "QueueTraits.hpp"
#include "VulkanFunctionLoader.hpp"
#include "version.hpp"

namespace vka {

class Surface;
class Device;

class Instance : std::enable_shared_from_this<Instance> {
public:
  Instance(
      const char* appName,
      uint32_t appMajorVersion,
      uint32_t appMinorVersion,
      uint32_t appPatchVersion,
      std::vector<const char*> instanceExtensions,
      std::vector<const char*> layers);

  std::shared_ptr<Surface> addSurface();
  std::shared_ptr<Device> addDevice();
  const std::vector<std::shared_ptr<Surface>>& getSurfaces() {
    return surfaces;
  }
  const std::vector<std::shared_ptr<Device>>& getDevices() { return devices; }
  VkInstance getHandle() { return instanceHandle; }

private:
  const char* appName;
  uint32_t appMajorVersion;
  uint32_t appMinorVersion;
  uint32_t appPatchVersion;
  std::vector<const char*> instanceExtensions;
  std::vector<const char*> layers;
  LibraryHandle vulkanLibrary;
  VkInstance instanceHandle;
  std::vector<std::shared_ptr<Surface>> surfaces;
  std::vector<VkPhysicalDevice> physicalDevices;
  std::vector<std::shared_ptr<Device>> devices;
};
}  // namespace vka