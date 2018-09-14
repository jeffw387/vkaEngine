#include "Instance.hpp"
#include "VulkanFunctionLoader.hpp"
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "Device.hpp"
#include <memory>
#include "Engine.hpp"
#include "Config.hpp"

namespace vka {

static VkBool32 vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
  auto multilogger = reinterpret_cast<spdlog::logger*>(pUserData);
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    multilogger->error(
        "Vulkan Error: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
  } else if (
      messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    multilogger->warn(
        "Vulkan Warning: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
  } else {
    multilogger->info(
        "Vulkan Info: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
  }
  return VK_FALSE;
}

Device::Device(Instance* instance, DeviceRequirements requirements)
    : instance(instance), requirements(requirements) {
  multilogger = spdlog::get(LoggerName);
  multilogger->info("Creating device.");
  physicalDeviceHandle = getInstance()->physicalDevices.at(0);
  vkGetPhysicalDeviceProperties(physicalDeviceHandle, &deviceProperties);

  uint32_t queueFamilyPropertyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDeviceHandle, &queueFamilyPropertyCount, nullptr);
  queueFamilyProperties.resize(queueFamilyPropertyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDeviceHandle,
      &queueFamilyPropertyCount,
      queueFamilyProperties.data());

  uint32_t currentFamilyIndex{};
  for (const auto& familyProps : queueFamilyProperties) {
    if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueueIndex = currentFamilyIndex;
      break;
    }
    ++currentFamilyIndex;
  }
  if (graphicsQueueIndex == U32Max) {
    multilogger->critical("No graphics-capable queue found!");
  }

  vkGetPhysicalDeviceMemoryProperties(physicalDeviceHandle, &memoryProperties);

  VkPhysicalDeviceFeatures enabledFeatures{};
  for (auto feature : requirements.requiredFeatures) {
    switch (feature) {
      case PhysicalDeviceFeatures::robustBufferAccess:
        enabledFeatures.robustBufferAccess = true;
        break;
      case PhysicalDeviceFeatures::geometryShader:
        enabledFeatures.geometryShader = true;
        break;
      case PhysicalDeviceFeatures::multiDrawIndirect:
        enabledFeatures.multiDrawIndirect = true;
        break;
      case PhysicalDeviceFeatures::drawIndirectFirstInstance:
        enabledFeatures.drawIndirectFirstInstance = true;
        break;
      case PhysicalDeviceFeatures::fillModeNonSolid:
        enabledFeatures.fillModeNonSolid = true;
        break;
      case PhysicalDeviceFeatures::multiViewport:
        enabledFeatures.multiViewport = true;
        break;
      case PhysicalDeviceFeatures::samplerAnistropy:
        enabledFeatures.samplerAnisotropy = true;
        break;
    }
  }

  auto findGraphicsQueueFamily = [&]() {
    for (uint32_t i = 0U; i < queueFamilyPropertyCount; ++i) {
      if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
          VK_QUEUE_GRAPHICS_BIT) {
        multilogger->info("Graphics queue family index selected: {}", i);
        return i;
      }
    }
    auto errorMsg = "Cannot find graphics queue family.";
    multilogger->critical(errorMsg);
    multilogger->flush();
    throw std::runtime_error(errorMsg);
  };

  float queuePriority = 1.f;
  queueCreateInfo.pQueuePriorities = &queuePriority;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.queueFamilyIndex = findGraphicsQueueFamily();

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(requirements.deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames =
      requirements.deviceExtensions.data();
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  auto deviceResult = vkCreateDevice(
      physicalDeviceHandle, &deviceCreateInfo, nullptr, &deviceHandle);
  if (deviceResult != VK_SUCCESS) {
    multilogger->error("Device not created, result code {}.", deviceResult);
  }
  deviceOwner = DeviceOwner(deviceHandle);

  LoadDeviceLevelEntryPoints(deviceHandle);

  vkGetDeviceQueue(deviceHandle, graphicsQueueIndex, 0, &graphicsQueue);

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.physicalDevice = physicalDeviceHandle;
  allocatorCreateInfo.device = deviceHandle;
  vmaCreateAllocator(&allocatorCreateInfo, &allocator);
  allocatorOwner = AllocatorOwner(allocator);

  VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
  messengerCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  messengerCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  messengerCreateInfo.pfnUserCallback = vulkanDebugCallback;
  messengerCreateInfo.pUserData = multilogger.get();

  if (vkCreateDebugUtilsMessengerEXT) {
    vkCreateDebugUtilsMessengerEXT(
        getInstance()->getHandle(),
        &messengerCreateInfo,
        nullptr,
        &debugMessenger);
    debugMessengerOwner =
        DebugMessengerOwner(debugMessenger, getInstance()->getHandle());
  }
}
}  // namespace vka