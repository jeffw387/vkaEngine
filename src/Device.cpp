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
#include "Logger.hpp"
#include <fstream>

namespace vka {
PhysicalDeviceData::PhysicalDeviceData(VkInstance instance) {
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
  physicalDevices.resize(physicalDeviceCount);
  vkEnumeratePhysicalDevices(
      instance, &physicalDeviceCount, physicalDevices.data());

  for (const auto& physicalDevice : physicalDevices) {
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice, &deviceMemoryProperties);

    std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties{};
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    deviceQueueFamilyProperties.resize(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyPropertyCount,
        deviceQueueFamilyProperties.data());

    properties[physicalDevice] = std::move(deviceProperties);
    memoryProperties[physicalDevice] = std::move(deviceMemoryProperties);
    queueFamilyProperties[physicalDevice] =
        std::move(deviceQueueFamilyProperties);
  }
}
Device::Device(
    VkInstance instance,
    VkSurfaceKHR surface,
    std::vector<const char*> deviceExtensions,
    std::vector<PhysicalDeviceFeatures> enabledFeatures,
    DeviceSelectCallback selectCallback)
    : surface(surface), physicalDeviceData(instance) {
  MultiLogger::get()->info("Creating device.");

  physicalDeviceHandle = selectCallback(physicalDeviceData);
  deviceProperties = physicalDeviceData.properties.at(physicalDeviceHandle);
  memoryProperties =
      physicalDeviceData.memoryProperties.at(physicalDeviceHandle);
  queueFamilyProperties =
      physicalDeviceData.queueFamilyProperties.at(physicalDeviceHandle);

  graphicsQueueIndex = [&]() {
    for (uint32_t i = 0U; i < queueFamilyProperties.size(); ++i) {
      if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
          VK_QUEUE_GRAPHICS_BIT) {
        MultiLogger::get()->info("Graphics queue family index selected: {}", i);
        return i;
      }
    }
    auto errorMsg = "Cannot find graphics queue family.";
    MultiLogger::get()->critical(errorMsg);
    MultiLogger::get()->flush();
    throw std::runtime_error(errorMsg);
  }();

  float queuePriority = 1.f;
  queueCreateInfo.pQueuePriorities = &queuePriority;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;

  auto enabledFeaturesVk = makeVkFeatures(enabledFeatures);
  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pEnabledFeatures = &enabledFeaturesVk;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  auto deviceResult = vkCreateDevice(
      physicalDeviceHandle, &deviceCreateInfo, nullptr, &deviceHandle);
  if (deviceResult != VK_SUCCESS) {
    MultiLogger::get()->error(
        "Device not created, result code {}.", deviceResult);
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

UniqueAllocatedImage Device::createAllocatedImage2D(
    VkExtent2D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    ImageAspect aspect) {
  VkImageCreateInfo imageCreateInfo{};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.usage = usage;
  imageCreateInfo.format = format;
  imageCreateInfo.extent = {extent.width, extent.height, 1};
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.pQueueFamilyIndices = &graphicsQueueIndex;
  imageCreateInfo.queueFamilyIndexCount = 1;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  AllocatedImage result{};
  vmaCreateImage(
      allocator,
      &imageCreateInfo,
      &allocationCreateInfo,
      &result.image,
      &result.allocation,
      &result.allocInfo);
  return UniqueAllocatedImage(result, {allocator});
}

UniqueImageView
Device::createImageView2D(VkImage image, VkFormat format, ImageAspect aspect) {
  VkImageViewCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.subresourceRange = {
      static_cast<VkImageAspectFlags>(aspect), 0, 1, 0, 1};
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.image = image;
  createInfo.format = format;

  VkImageView view{};
  vkCreateImageView(deviceHandle, &createInfo, nullptr, &view);
  return UniqueImageView(view, {deviceHandle});
}

Swapchain Device::createSwapchain(
    VkSwapchainKHR oldSwapchain,
    VkFormat format) {
  auto capabilities = getSurfaceCapabilities();

  MultiLogger::get()->info(
      "capabilities.minImageExtent: w{} h{}",
      capabilities.minImageExtent.width,
      capabilities.minImageExtent.height);
  MultiLogger::get()->info(
      "capabilities.currentExtent: w{} h{}",
      capabilities.currentExtent.width,
      capabilities.currentExtent.height);
  MultiLogger::get()->info(
      "capabilities.maxImageExtent: w{} h{}",
      capabilities.maxImageExtent.width,
      capabilities.maxImageExtent.height);
  MultiLogger::get()->info(
      "capabilities.minImageCount: {}", capabilities.minImageCount);
  MultiLogger::get()->info(
      "capabilities.maxImageCount: {}", capabilities.maxImageCount);
  MultiLogger::get()->info(
      "capabilities.currentTransform: {}", capabilities.currentTransform);
  for (const auto& [bit, name] : ImageUsageFlags) {
    MultiLogger::get()->info(
        "{} image usage supported: {}",
        name,
        ((bit & capabilities.supportedUsageFlags) == bit));
  }

  for (const auto& [bit, name] : CompositeAlphaFlags) {
    MultiLogger::get()->info(
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
    MultiLogger::get()->info("Supported present mode: {}.", PresentModes[mode]);
  }
  SwapchainCreateInfo createInfo{};
  createInfo.addQueueFamilyIndex(graphicsQueueIndex);
  createInfo.setImageExtent(capabilities.currentExtent);
  createInfo.setSurfacePreTransform(capabilities.currentTransform);
  createInfo.setSurface(surface);
  createInfo.setImageFormat(format);
  createInfo.setOldSwapchain(oldSwapchain);
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

std::unique_ptr<CommandPool> Device::createCommandPool() {
  return std::make_unique<CommandPool>(deviceHandle, gfxQueueIndex());
}

DescriptorPool Device::createDescriptorPool(
    std::vector<VkDescriptorPoolSize> poolSizes,
    uint32_t maxSets) {
  return DescriptorPool(deviceHandle, poolSizes, maxSets);
}

std::unique_ptr<DescriptorSetLayout> Device::createSetLayout(
    std::vector<VkDescriptorSetLayoutBinding> bindings) {
  return std::make_unique<DescriptorSetLayout>(deviceHandle, std::move(bindings));
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
    MultiLogger::get()->info("shader file length == {} bytes", fileLength);
    shaderFile.seekg(std::ios::beg);
    binaryData.resize(fileLength);
    shaderFile.read(binaryData.data(), fileLength);
    return ShaderModule(deviceHandle, binaryData);
  } catch (const std::exception& e) {
    MultiLogger::get()->critical("error while creating shader: {}", e.what());
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
    const std::vector<VkSemaphore>& signalSemaphores,
    VkFence fence) {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
  submitInfo.pCommandBuffers = commandBuffers.data();
  submitInfo.signalSemaphoreCount =
      static_cast<uint32_t>(signalSemaphores.size());
  submitInfo.pSignalSemaphores = signalSemaphores.data();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
}

RenderPass Device::createRenderPass(const VkRenderPassCreateInfo& createInfo) {
  return RenderPass(deviceHandle, createInfo);
}

void Device::waitIdle() { vkDeviceWaitIdle(deviceHandle); }
}  // namespace vka