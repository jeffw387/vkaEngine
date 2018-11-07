#include "Instance.hpp"
#include <GLFW/glfw3.h>
#include "Device.hpp"
#include "Surface.hpp"
#include <stdexcept>
#include <iostream>
#include "Engine.hpp"
#include "spdlog/spdlog.h"

namespace vka {
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

PFN_vkCreateDebugUtilsMessengerEXT Instance::vkCreateDebugUtilsMessengerEXT =
    {};
PFN_vkDestroyDebugUtilsMessengerEXT Instance::vkDestroyDebugUtilsMessengerEXT =
    {};

void DebugMessengerDeleter::operator()(VkDebugUtilsMessengerEXT messenger) {
  Instance::vkDestroyDebugUtilsMessengerEXT(instanceHandle, messenger, nullptr);
}

Instance::Instance(Engine* engine, InstanceCreateInfo instanceCreateInfo)
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

  // vulkanLibrary = LoadVulkanLibrary();
  // LoadExportedEntryPoints(vulkanLibrary);
  // LoadGlobalLevelEntryPoints();

  auto instanceResult = vkCreateInstance(&createInfo, nullptr, &instanceHandle);
  if (instanceResult != VK_SUCCESS) {
    // TODO: error handling
  }
  instanceOwner = InstanceOwner(instanceHandle);

  // LoadInstanceLevelEntryPoints(instanceHandle);
  Instance::vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(
          instanceHandle, "vkCreateDebugUtilsMessengerEXT");
  Instance::vkDestroyDebugUtilsMessengerEXT =
      (PFN_vkDestroyDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(
          instanceHandle, "vkDestroyDebugUtilsMessengerEXT");

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
  Instance::vkCreateDebugUtilsMessengerEXT(
      instanceHandle, &messengerCreateInfo, nullptr, &debugMessenger);
  debugMessengerOwner = DebugMessengerOwner(debugMessenger, instanceHandle);
}

Device* Instance::createDevice(
    std::vector<const char*> deviceExtensions,
    std::vector<PhysicalDeviceFeatures> enabledFeatures,
    DeviceSelectCallback selectCallback) {
  device = std::make_unique<Device>(
      instanceHandle,
      *surface,
      deviceExtensions,
      enabledFeatures,
      selectCallback);
  return device.get();
}

Surface* Instance::createSurface(SurfaceCreateInfo surfaceCreateInfo) {
  surface = std::make_unique<Surface>(instanceHandle, surfaceCreateInfo);
  return surface.get();
}

}  // namespace vka