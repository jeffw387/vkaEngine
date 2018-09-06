#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include "Surface.hpp"
#include <stdexcept>
#include <iostream>
#include "Engine.hpp"

namespace vka {
Instance::Instance(Engine::Ptr engine, InstanceCreateInfo instanceCreateInfo)
  : engine(engine), instanceCreateInfo(instanceCreateInfo) {
  glfwInit();
  glfwSetErrorCallback([](int error, const char* desc) { std::cerr << desc; });

  // TODO: could log if vulkan isn't supported then exit

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(
    EngineVersion.major, EngineVersion.minor, EngineVersion.patch);
  appInfo.applicationVersion = VK_MAKE_VERSION(
    instanceCreateInfo.appVersion.major,
    instanceCreateInfo.appVersion.minor,
    instanceCreateInfo.appVersion.patch);
  appInfo.pEngineName = "vkaEngine";
  appInfo.pApplicationName = instanceCreateInfo.appName;

  uint32_t glfwRequiredInstanceExtensionCount{};
  auto glfwRequiredInstanceExtensions =
    glfwGetRequiredInstanceExtensions(&glfwRequiredInstanceExtensionCount);
  for (auto i = 0U; i < glfwRequiredInstanceExtensionCount; ++i) {
    instanceCreateInfo.instanceExtensions.push_back(
      glfwRequiredInstanceExtensions[i]);
  }
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.enabledExtensionCount =
    static_cast<uint32_t>(instanceCreateInfo.instanceExtensions.size());
  createInfo.ppEnabledExtensionNames =
    instanceCreateInfo.instanceExtensions.data();
  createInfo.enabledLayerCount =
    static_cast<uint32_t>(instanceCreateInfo.layers.size());
  createInfo.ppEnabledLayerNames = instanceCreateInfo.layers.data();
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

std::shared_ptr<Device> Instance::createDevice(
  DeviceRequirements requirements) {
  device = std::make_shared<Device>(shared_from_this(), requirements);
  return device;
}

std::shared_ptr<Surface> Instance::createSurface(
  SurfaceCreateInfo surfaceCreateInfo) {
  surface = std::make_shared<Surface>(shared_from_this(), surfaceCreateInfo);
  return surface;
}

Instance::~Instance() { vkDestroyInstance(instanceHandle, nullptr); }
}  // namespace vka