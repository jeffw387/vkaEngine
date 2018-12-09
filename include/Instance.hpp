#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "version.hpp"
#include "spdlog/spdlog.h"
#include "Device.hpp"

namespace vka {

class Engine;
class Surface;
class Device;
struct SurfaceCreateInfo;

struct InstanceCreateInfo {
  const char* appName;
  Version appVersion;
  std::vector<const char*> instanceExtensions;
  std::vector<const char*> layers;
};

class Instance {
public:
  Instance(Engine*, InstanceCreateInfo);
  ~Instance();

  std::unique_ptr<Device> createDevice(
      VkSurfaceKHR surface,
      std::vector<const char*> deviceExtensions,
      std::vector<PhysicalDeviceFeatures> enabledFeatures,
      DeviceSelectCallback selectCallback);
  std::unique_ptr<Surface> createSurface(SurfaceCreateInfo);
  operator VkInstance() { return instance; }

private:
  Engine* engine;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

public:
  static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
  static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
};
}  // namespace vka