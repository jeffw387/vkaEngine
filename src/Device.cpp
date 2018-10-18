#include "Device.hpp"
#include "Instance.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <memory>
#include "Engine.hpp"
#include "Config.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include <fstream>

namespace vka {
Device::Device(
    VkInstance instance,
    VkSurfaceKHR surface,
    DeviceRequirements requirements)
    : surface(surface), requirements(requirements) {
  multilogger = spdlog::get(LoggerName);
  multilogger->info("Creating device.");

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
  physicalDevices.resize(physicalDeviceCount);
  vkEnumeratePhysicalDevices(
      instance, &physicalDeviceCount, physicalDevices.data());

  physicalDeviceHandle = physicalDevices.at(0);

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
  if (graphicsQueueIndex == UINT32_MAX) {
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

  // LoadDeviceLevelEntryPoints(deviceHandle);

  vkGetDeviceQueue(deviceHandle, graphicsQueueIndex, 0, &graphicsQueue);

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.physicalDevice = physicalDeviceHandle;
  allocatorCreateInfo.device = deviceHandle;
  vmaCreateAllocator(&allocatorCreateInfo, &allocator);
  allocatorOwner = AllocatorOwner(allocator);
}

VkSurfaceCapabilitiesKHR Device::getSurfaceCapabilities() {
  VkSurfaceCapabilitiesKHR capabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDeviceHandle, surface, &capabilities);
  return capabilities;
}

UniqueAllocatedBuffer Device::createAllocatedBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage) {
  AllocatedBuffer result{};
  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.usage = usage;
  bufferCreateInfo.queueFamilyIndexCount = 1;
  bufferCreateInfo.pQueueFamilyIndices = &graphicsQueueIndex;
  bufferCreateInfo.size = size;

  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = memoryUsage;
  vmaCreateBuffer(
      allocator,
      &bufferCreateInfo,
      &allocationCreateInfo,
      &result.buffer,
      &result.allocation,
      &result.allocInfo);
  auto bufferUnique =
      UniqueAllocatedBuffer(result, AllocatedBufferDeleter{allocator});
  return bufferUnique;
}

Swapchain Device::createSwapchain() {
  auto capabilities = getSurfaceCapabilities();

  multilogger->info(
      "capabilities.minImageExtent: w{} h{}",
      capabilities.minImageExtent.width,
      capabilities.minImageExtent.height);
  multilogger->info(
      "capabilities.currentExtent: w{} h{}",
      capabilities.currentExtent.width,
      capabilities.currentExtent.height);
  multilogger->info(
      "capabilities.maxImageExtent: w{} h{}",
      capabilities.maxImageExtent.width,
      capabilities.maxImageExtent.height);
  multilogger->info(
      "capabilities.minImageCount: {}", capabilities.minImageCount);
  multilogger->info(
      "capabilities.maxImageCount: {}", capabilities.maxImageCount);
  multilogger->info(
      "capabilities.currentTransform: {}", capabilities.currentTransform);
  for (const auto& [bit, name] : ImageUsageFlags) {
    multilogger->info(
        "{} image usage supported: {}",
        name,
        ((bit & capabilities.supportedUsageFlags) == bit));
  }

  for (const auto& [bit, name] : CompositeAlphaFlags) {
    multilogger->info(
        "{} supported: {}",
        name,
        ((bit & capabilities.supportedCompositeAlpha) == bit));
  }

  uint32_t presentModeCount{};
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDeviceHandle, surface, &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> supportedModes;
  supportedModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDeviceHandle, surface, &presentModeCount, supportedModes.data());
  for (const auto& mode : supportedModes) {
    multilogger->info("Supported present mode: {}.", PresentModes[mode]);
  }
  SwapchainCreateInfo createInfo{};
  createInfo.addQueueFamilyIndex(graphicsQueueIndex);
  createInfo.setImageExtent(capabilities.currentExtent);
  createInfo.setSurfacePreTransform(capabilities.currentTransform);
  createInfo.setSurface(surface);
  return Swapchain(
      physicalDeviceHandle, deviceHandle, graphicsQueueIndex, createInfo);
}

GraphicsPipeline Device::createGraphicsPipeline(
    VkPipelineCache pipelineCache,
    const VkGraphicsPipelineCreateInfo& createInfo) {
  return GraphicsPipeline(deviceHandle, pipelineCache, createInfo);
}

ComputePipeline Device::createComputePipeline(
    VkPipelineCache pipelineCache,
    const VkComputePipelineCreateInfo& createInfo) {
  return ComputePipeline(deviceHandle, pipelineCache, createInfo);
}

CommandPool Device::createCommandPool() {
  return CommandPool(deviceHandle, gfxQueueIndex());
}

DescriptorPool Device::createDescriptorPool(
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets) {
  return DescriptorPool(deviceHandle, poolSizes, maxSets);
}

DescriptorSetLayout Device::createSetLayout(
    std::vector<VkDescriptorSetLayoutBinding> bindings) {
  return DescriptorSetLayout(deviceHandle, std::move(bindings));
}

PipelineLayout Device::createPipelineLayout(
    std::vector<VkPushConstantRange> pushRanges,
    std::vector<VkDescriptorSetLayout> setLayouts) {
  return PipelineLayout(
      deviceHandle, std::move(pushRanges), std::move(setLayouts));
}

ShaderModule Device::createShaderModule(std::string shaderPath) {
  try {
    std::vector<char> binaryData;
    std::ifstream shaderFile(
        shaderPath,
        std::ios_base::binary | std::ios_base::ate | std::ios_base::in);
    auto fileLength = shaderFile.tellg();
    multilogger->info("shader file length == {} bytes", fileLength);
    shaderFile.seekg(std::ios::beg);
    binaryData.resize(fileLength);
    shaderFile.read(binaryData.data(), fileLength);
    return ShaderModule(deviceHandle, binaryData);
  } catch (const std::exception& e) {
    multilogger->critical("error while creating shader: {}", e.what());
    throw e;
  }
}

UniqueFramebuffer Device::createFramebuffer(
    std::vector<VkImageView> attachments,
    VkRenderPass renderPass,
    uint32_t width,
    uint32_t height) {
  VkFramebuffer framebuffer{};
  VkFramebufferCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  createInfo.pAttachments = attachments.data();
  createInfo.renderPass = renderPass;
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = 1;
  vkCreateFramebuffer(deviceHandle, &createInfo, nullptr, &framebuffer);
  return UniqueFramebuffer(framebuffer, {deviceHandle});
}

Fence Device::createFence(bool signaled) {
  return Fence(deviceHandle, signaled);
}

Semaphore Device::createSemaphore() { return Semaphore(deviceHandle); }

VkResult Device::presentImage(
    VkSwapchainKHR swapchain,
    uint32_t imageIndex,
    VkSemaphore waitSemaphore) {
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &waitSemaphore;
  return vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

void Device::queueSubmit(
    const std::vector<VkSemaphore>& waitSemaphores,
    const std::vector<VkCommandBuffer>& commandBuffers,
    const std::vector<VkSemaphore>& signalSemaphores) {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
  submitInfo.pCommandBuffers = commandBuffers.data();
  submitInfo.signalSemaphoreCount =
      static_cast<uint32_t>(signalSemaphores.size());
  submitInfo.pSignalSemaphores = signalSemaphores.data();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, 0);
}

RenderPass Device::createRenderPass(const VkRenderPassCreateInfo& createInfo) {
  return RenderPass(deviceHandle, createInfo);
}

void Device::waitIdle() { vkDeviceWaitIdle(deviceHandle); }
}  // namespace vka