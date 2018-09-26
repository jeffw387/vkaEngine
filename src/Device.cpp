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

AllocatedBuffer Device::createAllocatedBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaAllocationCreateFlags allocFlags,
    VmaMemoryUsage memoryUsage) {
  AllocatedBuffer result{};
  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.usage = usage;
  bufferCreateInfo.queueFamilyIndexCount = 1;
  bufferCreateInfo.pQueueFamilyIndices = &graphicsQueueIndex;
  bufferCreateInfo.size = size;

  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.flags = allocFlags;
  allocationCreateInfo.usage = memoryUsage;
  vmaCreateBuffer(
      allocator,
      &bufferCreateInfo,
      &allocationCreateInfo,
      &result.buffer,
      &result.allocation,
      &result.allocInfo);
  auto bufferUnique =
      UniqueAllocatedBuffer(result, AllocatedBufferDeleter(allocator));
  allocatedBuffers.push_back(std::move(bufferUnique));
  return result;
}

Swapchain* Device::createSwapchain() {
  auto capabilities = getSurfaceCapabilities();

  SwapchainCreateInfo createInfo{};
  createInfo.addQueueFamilyIndex(graphicsQueueIndex);
  createInfo.setImageExtent(capabilities.currentExtent);
  createInfo.setSurfacePreTransform(capabilities.currentTransform);
  createInfo.setSurface(surface);
  swapchain = std::make_unique<Swapchain>(deviceHandle, createInfo);
  return swapchain.get();
}

GraphicsPipeline* Device::createGraphicsPipeline(
    const VkGraphicsPipelineCreateInfo& createInfo) {
  graphicsPipelines.push_back(std::make_unique<GraphicsPipeline>(
      deviceHandle, *pipelineCache, createInfo));
  return graphicsPipelines.back().get();
}

ComputePipeline* Device::createComputePipeline(
    const VkComputePipelineCreateInfo& createInfo) {
  computePipelines.push_back(std::make_unique<ComputePipeline>(
      deviceHandle, *pipelineCache, createInfo));
  return computePipelines.back().get();
}

CommandPool* Device::createCommandPool() {
  commandPools.emplace_back(
      std::make_unique<CommandPool>(deviceHandle, gfxQueueIndex()));
  return commandPools.back().get();
}

DescriptorPool* Device::createDescriptorPool(
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets) {
  descriptorPools.push_back(
      std::make_unique<DescriptorPool>(deviceHandle, poolSizes, maxSets));
  return descriptorPools.back().get();
}

DescriptorSetLayout* Device::createSetLayout(
    const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
  descriptorSetLayouts.push_back(
      std::make_unique<DescriptorSetLayout>(deviceHandle, bindings));
  return descriptorSetLayouts.back().get();
}

PipelineLayout* Device::createPipelineLayout(
    const std::vector<VkPushConstantRange>& pushRanges,
    const std::vector<VkDescriptorSetLayout>& setLayouts) {
  pipelineLayouts.push_back(
      std::make_unique<PipelineLayout>(deviceHandle, pushRanges, setLayouts));
  return pipelineLayouts.back().get();
}

ShaderModule* Device::createShaderModule(std::string shaderPath) {
  std::vector<uint32_t> binaryData;
  std::basic_ifstream<uint32_t> shaderFile(
      shaderPath,
      std::ios_base::binary | std::ios_base::ate | std::ios_base::in);
  auto fileLength = shaderFile.tellg();
  binaryData.resize(fileLength);
  shaderFile.read(binaryData.data(), fileLength);
  shaderModules.push_back(
      std::make_unique<ShaderModule>(deviceHandle, binaryData));
  return shaderModules.back().get();
}

VkResult Device::presentImage(uint32_t imageIndex, VkSemaphore waitSemaphore) {
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = *swapchain;
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

RenderPass* Device::createRenderPass(const VkRenderPassCreateInfo& createInfo) {
  renderPasses.push_back(
      std::make_unique<RenderPass>(deviceHandle, createInfo));
  return renderPasses.back().get();
}

void Device::waitIdle() { vkDeviceWaitIdle(deviceHandle); }
}  // namespace vka