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

struct InstanceDeleter {
  using pointer = VkInstance;
  void operator()(VkInstance instance) { vkDestroyInstance(instance, nullptr); }
};
using InstanceOwner = std::unique_ptr<VkInstance, InstanceDeleter>;

struct DebugMessengerDeleter {
  using pointer = VkDebugUtilsMessengerEXT;
  VkInstance instanceHandle;
  DebugMessengerDeleter() = default;
  DebugMessengerDeleter(std::nullptr_t) : instanceHandle(0) {}
  DebugMessengerDeleter(VkInstance instanceHandle)
      : instanceHandle(instanceHandle) {}
  void operator()(VkDebugUtilsMessengerEXT messenger);
};
using DebugMessengerOwner =
    std::unique_ptr<VkDebugUtilsMessengerEXT, DebugMessengerDeleter>;

class Instance {
  friend class Device;

public:
  Instance() = default;
  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&);
  Instance(Engine*, InstanceCreateInfo);
  Instance(Instance&&) = default;
  Instance& operator=(Instance&&) = default;
  ~Instance() = default;

  Device* createDevice(
      std::vector<const char*> deviceExtensions,
      std::vector<PhysicalDeviceFeatures> enabledFeatures,
      DeviceSelectCallback selectCallback);
  Device* getDevice() { return device.get(); }
  Surface* createSurface(SurfaceCreateInfo);
  Surface* getSurface() { return surface.get(); }
  operator VkInstance() { return instanceHandle; }

private:
  Engine* engine;
  VkInstance instanceHandle;
  InstanceOwner instanceOwner;
  VkDebugUtilsMessengerEXT debugMessenger;
  DebugMessengerOwner debugMessengerOwner;
  std::unique_ptr<Surface> surface;
  std::unique_ptr<Device> device;
};
}  // namespace vka