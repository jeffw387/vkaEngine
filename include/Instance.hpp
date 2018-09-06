#pragma once

#include "VulkanFunctionLoader.hpp"
#include <memory>
#include <vector>
#include "version.hpp"

namespace vka {

class Engine;
class Surface;
class Device;
struct DeviceRequirements;
struct SurfaceCreateInfo;

struct InstanceCreateInfo {
  const char* appName;
  Version appVersion;
  std::vector<const char*> instanceExtensions;
  std::vector<const char*> layers;
};

class Instance : public std::enable_shared_from_this<Instance> {
  friend class Device;

public:
  using Ptr = std::shared_ptr<Instance>;

  Instance() = delete;
  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&);
  Instance(std::shared_ptr<Engine>, InstanceCreateInfo);
  Instance(Instance&&) = default;
  Instance& operator=(Instance&&) = default;
  ~Instance();

  std::shared_ptr<Device> createDevice(DeviceRequirements);
  std::shared_ptr<Surface> createSurface(SurfaceCreateInfo);
  std::shared_ptr<Surface> getSurface() { return surface; }
  std::shared_ptr<Device> getDevice() { return device; }
  VkInstance getHandle() { return instanceHandle; }
  std::shared_ptr<Engine> getEngine() { return engine.lock(); }

private:
  std::weak_ptr<Engine> engine;
  InstanceCreateInfo instanceCreateInfo;
  LibraryHandle vulkanLibrary;
  VkInstance instanceHandle;
  std::shared_ptr<Surface> surface;
  std::vector<VkPhysicalDevice> physicalDevices;
  std::shared_ptr<Device> device;
};
}  // namespace vka