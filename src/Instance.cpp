#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include "Surface.hpp"
#include <stdexcept>
#include <iostream>
#include "Engine.hpp"
#include "spdlog/spdlog.h"

namespace vka {
Instance::Instance(Engine*, InstanceCreateInfo instanceCreateInfo)
    : engine(engine), instanceCreateInfo(instanceCreateInfo) {
  multilogger = spdlog::get(LoggerName);
  multilogger->info("Creating instance.");
  glfwSetErrorCallback([](int error, const char* desc) {
    auto multilogger = spdlog::get(LoggerName);
    multilogger->error("GLFW error {}: {}", error, desc);
  });

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

  auto instanceResult = vkCreateInstance(&createInfo, nullptr, &instanceHandle);
  if (instanceResult != VK_SUCCESS) {
    // TODO: error handling
  }
  instanceOwner = InstanceOwner(instanceHandle);

  LoadInstanceLevelEntryPoints(instanceHandle);

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, nullptr);
  physicalDevices.resize(physicalDeviceCount);
  vkEnumeratePhysicalDevices(
      instanceHandle, &physicalDeviceCount, physicalDevices.data());
}

Device* Instance::createDevice(DeviceRequirements requirements) {
  device = std::make_unique<Device>(this, requirements);
  return device.get();
}

Surface* Instance::createSurface(SurfaceCreateInfo surfaceCreateInfo) {
  surface = std::make_unique<Surface>(this, surfaceCreateInfo);
  return surface.get();
}

}  // namespace vka