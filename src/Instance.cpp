#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include "Surface.hpp"

namespace vka {
Instance::Instance(Instance::CreateInfo instanceCreateInfo)
    : instanceCreateInfo(instanceCreateInfo) {
  glfwInit();

  // TODO: could log if vulkan isn't supported then exit

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(
      EngineVersionMajor, EngineVersionMinor, EngineVersionPatch);
  appInfo.applicationVersion = VK_MAKE_VERSION(
      instanceCreateInfo.appMajorVersion,
      instanceCreateInfo.appMinorVersion,
      instanceCreateInfo.appPatchVersion);
  appInfo.pEngineName = "vkaEngine";
  appInfo.pApplicationName = instanceCreateInfo.appName;

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

std::shared_ptr<Device> Instance::addDevice(
    Device::Requirements* requirements) {
  devices.push_back(
      std::make_shared<Device>(shared_from_this(), *requirements));
  return devices.back();
}

std::shared_ptr<Surface> Instance::createSurface(
    Surface::CreateInfo surfaceCreateInfo) {
  surface = std::make_shared<Surface>(shared_from_this(), surfaceCreateInfo);
  return surface;
}

}  // namespace vka