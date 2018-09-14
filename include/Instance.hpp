#pragma once

#include "VulkanFunctionLoader.hpp"
#include <memory>
#include <vector>
#include "version.hpp"
#include "spdlog/spdlog.h"
#include <GLFW/glfw3.h>

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

struct InstanceDeleter {
  using pointer = VkInstance;
  void operator()(VkInstance instance) { vkDestroyInstance(instance, nullptr); }
};
using InstanceOwner = std::unique_ptr<VkInstance, InstanceDeleter>;

struct GLFWOwner {
  GLFWOwner() { glfwInit(); }

  GLFWOwner(GLFWOwner&&) = default;
  GLFWOwner& operator=(GLFWOwner&&) = default;
  GLFWOwner(const GLFWOwner&) = delete;
  GLFWOwner& operator=(const GLFWOwner&) = delete;
  ~GLFWOwner() { glfwTerminate(); }
};

class Instance {
  friend class Device;

public:
  Instance() = delete;
  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&);
  Instance(Engine*, InstanceCreateInfo);
  Instance(Instance&&) = default;
  Instance& operator=(Instance&&) = default;
  ~Instance() = default;

  Device* createDevice(DeviceRequirements);
  Device* getDevice() { return device.get(); }
  Engine* getEngine() { return engine; }
  Surface* createSurface(SurfaceCreateInfo);
  Surface* getSurface() { return surface.get(); }
  VkInstance getHandle() { return instanceHandle; }

  std::shared_ptr<spdlog::logger> multilogger;

private:
  Engine* engine;
  GLFWOwner glfwOwner;
  InstanceCreateInfo instanceCreateInfo;
  LibraryHandle vulkanLibrary;
  VkInstance instanceHandle;
  InstanceOwner instanceOwner;
  std::unique_ptr<Surface> surface;
  std::vector<VkPhysicalDevice> physicalDevices;
  std::unique_ptr<Device> device;
};
}  // namespace vka