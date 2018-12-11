#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "version.hpp"
#include "spdlog/spdlog.h"
#include "Device.hpp"
#include "Surface.hpp"

namespace vka {

struct InstanceCreateInfo {
  const char* appName;
  Version appVersion;
  std::vector<const char*> instanceExtensions;
  std::vector<const char*> layers;
};

template <typename PlatformT>
class Instance {
public:
  Instance(std::shared_ptr<PlatformT> platform, InstanceCreateInfo);
  ~Instance();

  std::unique_ptr<Surface> createSurface(SurfaceCreateInfo);
  std::unique_ptr<Device> createDevice(
      Surface* surface,
      std::vector<const char*> deviceExtensions,
      std::vector<PhysicalDeviceFeatures> enabledFeatures,
      DeviceSelectCallback selectCallback);
  operator VkInstance() { return instance; }

private:
  std::shared_ptr<PlatformT> platform;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

public:
  static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
  static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
};
}  // namespace vka