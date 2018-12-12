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

static VkBool32 vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    MultiLogger::get()->error(
        "Vulkan Error: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
    for (size_t objIndex{}; objIndex < pCallbackData->objectCount; ++objIndex) {
      if (pCallbackData->pObjects[objIndex].pObjectName != nullptr) {
        MultiLogger::get()->error(
            "Involved object: {}",
            pCallbackData->pObjects[objIndex].pObjectName);
      }
    }
  } else if (
      messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    MultiLogger::get()->warn(
        "Vulkan Warning: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
  } else {
    MultiLogger::get()->info(
        "Vulkan Info: {} {}",
        pCallbackData->pMessageIdName,
        pCallbackData->pMessage);
  }
  return VK_FALSE;
}

template <typename PlatformT>
class Instance {
public:
  Instance(std::shared_ptr<PlatformT> platform, InstanceCreateInfo instanceCreateInfo)
    : engine(engine) {
  MultiLogger::get()->info("Creating instance.");

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

  auto instanceResult = vkCreateInstance(&createInfo, nullptr, &instance);
  if (instanceResult != VK_SUCCESS) {
    // TODO: error handling
  }

  vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(
          instance, "vkCreateDebugUtilsMessengerEXT");
  vkDestroyDebugUtilsMessengerEXT =
      (PFN_vkDestroyDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(
          instance, "vkDestroyDebugUtilsMessengerEXT");

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
  vkCreateDebugUtilsMessenger(
      instance, &messengerCreateInfo, nullptr, &debugMessenger);
}
  ~Instance() {
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  vkDestroyInstance(instance, nullptr);
  }

  std::unique_ptr<Surface<PlatformT>> createSurface(SurfaceCreateInfo) {
    return std::make_unique<Surface<PlatformT>(engine, instance, surfaceCreateInfo);
  }
  std::unique_ptr<Device> createDevice(
      Surface<PlatformT>* surface,
      std::vector<const char*> deviceExtensions,
      std::vector<PhysicalDeviceFeatures> enabledFeatures,
      DeviceSelectCallback selectCallback) {
      return std::make_unique<Device>(
        instance, surface, deviceExtensions, enabledFeatures, selectCallback);
    }
  operator VkInstance() { return instance; }

private:
  std::shared_ptr<PlatformT> platform;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
};
}  // namespace vka