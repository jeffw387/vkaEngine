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
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
namespace vka {
PFN_vkSetDebugUtilsObjectNameEXT Device::vkSetDebugUtilsObjectNameEXT = {};

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

  physicalDevice = selectCallback(physicalDeviceData);
  deviceProperties = physicalDeviceData.properties.at(physicalDevice);
  memoryProperties = physicalDeviceData.memoryProperties.at(physicalDevice);
  queueFamilyProperties =
      physicalDeviceData.queueFamilyProperties.at(physicalDevice);

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
  auto deviceResult =
      vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
  if (deviceResult != VK_SUCCESS) {
    MultiLogger::get()->error(
        "Device not created, result code {}.", deviceResult);
  }

  vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.physicalDevice = physicalDevice;
  allocatorCreateInfo.device = device;
  vmaCreateAllocator(&allocatorCreateInfo, &allocator);

  vkSetDebugUtilsObjectNameEXT =
      (PFN_vkSetDebugUtilsObjectNameEXT)glfwGetInstanceProcAddress(
          instance, "vkSetDebugUtilsObjectNameEXT");
}

VkSurfaceCapabilitiesKHR Device::getSurfaceCapabilities() {
  VkSurfaceCapabilitiesKHR capabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &capabilities);
  return capabilities;
}

std::shared_ptr<Buffer> Device::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    bool dedicated) {
  return std::make_shared<Buffer>(
      allocator,
      size,
      usage,
      memoryUsage,
      std::vector<uint32_t>{graphicsQueueIndex},
      dedicated);
}

std::shared_ptr<Image> Device::createImage2D(
    VkExtent2D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    ImageAspect aspect,
    bool dedicated) {
  std::vector<uint32_t> queueIndices = {graphicsQueueIndex};
  return std::make_shared<Image>(
      allocator,
      std::move(queueIndices),
      VkExtent3D{extent.width, extent.height, 1U},
      std::move(format),
      std::move(usage),
      std::move(aspect),
      dedicated);
}

std::unique_ptr<ImageView>
Device::createImageView2D(VkImage image, VkFormat format, ImageAspect aspect) {
  return std::make_unique<ImageView>(device, image, format, aspect);
}

std::unique_ptr<Sampler> Device::createSampler(
    VkFilter magFilter,
    VkFilter minFilter,
    VkSamplerMipmapMode mipmapMode,
    VkSamplerAddressMode U,
    VkSamplerAddressMode V,
    VkSamplerAddressMode W,
    float mipLodBias,
    VkBool32 anisotropyEnable,
    float maxAnisotropy,
    VkBool32 compareEnable,
    VkCompareOp compareOp,
    float minLod,
    float maxLod,
    VkBorderColor borderColor,
    VkBool32 unnormalizedCoordinates) {
  return std::make_unique<Sampler>(
      device,
      magFilter,
      minFilter,
      mipmapMode,
      U,
      V,
      W,
      mipLodBias,
      anisotropyEnable,
      maxAnisotropy,
      compareEnable,
      compareOp,
      minLod,
      maxLod,
      borderColor,
      unnormalizedCoordinates);
}

std::unique_ptr<Swapchain> Device::createSwapchain(
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
      physicalDevice, surface, &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> supportedModes;
  supportedModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &presentModeCount, supportedModes.data());
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
  return std::make_unique<Swapchain>(
      physicalDevice, device, graphicsQueueIndex, createInfo);
}
std::unique_ptr<PipelineCache> Device::createPipelineCache() {
  return std::make_unique<PipelineCache>(device);
}

std::unique_ptr<PipelineCache> Device::createPipelineCache(
    std::vector<char> initialData) {
  return std::make_unique<PipelineCache>(device, std::move(initialData));
}

std::shared_ptr<GraphicsPipeline> Device::createGraphicsPipeline(
    VkPipelineCache pipelineCache,
    const VkGraphicsPipelineCreateInfo& createInfo) {
  return std::make_shared<GraphicsPipeline>(device, pipelineCache, createInfo);
}

std::shared_ptr<ComputePipeline> Device::createComputePipeline(
    VkPipelineCache pipelineCache,
    const VkComputePipelineCreateInfo& createInfo) {
  return std::make_shared<ComputePipeline>(device, pipelineCache, createInfo);
}

std::unique_ptr<CommandPool> Device::createCommandPool(
    bool primary,
    bool transient) {
  return std::make_unique<CommandPool>(
      device,
      gfxQueueIndex(),
      primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
              : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      transient);
}

std::unique_ptr<DescriptorPool> Device::createDescriptorPool(
    std::vector<VkDescriptorPoolSize> poolSizes,
    uint32_t maxSets) {
  return std::make_unique<DescriptorPool>(
      device, std::move(poolSizes), maxSets);
}

std::unique_ptr<DescriptorSetLayout> Device::createSetLayout(
    std::vector<VkDescriptorSetLayoutBinding> bindings) {
  return std::make_unique<DescriptorSetLayout>(device, std::move(bindings));
}

std::shared_ptr<PipelineLayout> Device::createPipelineLayout(
    std::vector<VkPushConstantRange> pushRanges,
    std::vector<VkDescriptorSetLayout> setLayouts) {
  return std::make_shared<PipelineLayout>(
      device, std::move(pushRanges), std::move(setLayouts));
}

std::unique_ptr<ShaderModule> Device::createShaderModule(
    std::string shaderPath) {
  auto cwd = fs::current_path();
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
    return std::make_unique<ShaderModule>(device, binaryData);
  } catch (const std::exception& e) {
    MultiLogger::get()->critical("error while creating shader: {}", e.what());
    throw e;
  }
}

std::shared_ptr<Framebuffer> Device::createFramebuffer(
    VkRenderPass renderPass,
    std::vector<std::shared_ptr<ImageView>> attachments,
    VkExtent2D extent) {
  return std::make_shared<Framebuffer>(
      device, renderPass, std::move(attachments), extent);
}

std::unique_ptr<Fence> Device::createFence(bool signaled) {
  return std::make_unique<Fence>(device, signaled);
}

std::unique_ptr<Semaphore> Device::createSemaphore() {
  return std::make_unique<Semaphore>(device);
}

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
    std::vector<std::shared_ptr<CommandBuffer>> commandBuffers,
    const std::vector<VkSemaphore>& signalSemaphores,
    vka::Fence* fence) {
  std::vector<VkCommandBuffer> vkCmdBuffers;
  vkCmdBuffers.reserve(commandBuffers.size());
  for (auto& cmd : commandBuffers) {
    cmd->checkExecutable();
    vkCmdBuffers.push_back(*cmd);
    fence->subscribe([=]() { cmd->cmdExecuted(); });
    cmd->cmdPending();
  }
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.commandBufferCount = static_cast<uint32_t>(vkCmdBuffers.size());
  submitInfo.pCommandBuffers = vkCmdBuffers.data();
  submitInfo.signalSemaphoreCount =
      static_cast<uint32_t>(signalSemaphores.size());
  submitInfo.pSignalSemaphores = signalSemaphores.data();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, *fence);
}

std::shared_ptr<RenderPass> Device::createRenderPass(
    const VkRenderPassCreateInfo& createInfo) {
  return std::make_shared<RenderPass>(device, createInfo);
}

void Device::waitIdle() { vkDeviceWaitIdle(device); }

Device::~Device() {
  vmaDestroyAllocator(allocator);
  vkDestroyDevice(device, nullptr);
}
}  // namespace vka