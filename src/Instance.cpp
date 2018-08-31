#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"

namespace vka {
Instance::Instance(
    const char* appName,
    uint32_t appMajorVersion,
    uint32_t appMinorVersion,
    uint32_t appPatchVersion,
    std::vector<const char*> instanceExtensions,
    std::vector<const char*> layers)
    : appName(appName),
      appMajorVersion(appMajorVersion),
      appMinorVersion(appMinorVersion),
      appPatchVersion(appPatchVersion),
      instanceExtensions(instanceExtensions),
      layers(layers) {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(
      EngineVersionMajor, EngineVersionMinor, EngineVersionPatch);
  appInfo.applicationVersion =
      VK_MAKE_VERSION(appMajorVersion, appMinorVersion, appPatchVersion);
  appInfo.pEngineName = "vkaEngine";
  appInfo.pApplicationName = appName;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(instanceExtensions.size());
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();
  createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  createInfo.ppEnabledLayerNames = layers.data();
  createInfo.pApplicationInfo = &appInfo;

  vulkanLibrary = LoadVulkanLibrary();
  LoadExportedEntryPoints(vulkanLibrary);
  LoadGlobalLevelEntryPoints();
  // TODO: logging
  vkCreateInstance(&createInfo, nullptr, &instanceHandle);

  LoadInstanceLevelEntryPoints(instanceHandle);

  uint32_t physicalDeviceCount = 0;
  // TODO: logging
  vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, nullptr);
  physicalDevices.resize(physicalDeviceCount);
  // TODO: logging
  vkEnumeratePhysicalDevices(
      instanceHandle, &physicalDeviceCount, physicalDevices.data());
}

std::shared_ptr<Device> Instance::addDevice() {
  devices.push_back(std::make_shared<Device>(shared_from_this()));
  return devices.back();
}

std::shared_ptr<Surface> Instance::addSurface() {
  surfaces.push_back(std::make_shared<Surface>(shared_from_this()));
  return surfaces.back();
}
}  // namespace vka