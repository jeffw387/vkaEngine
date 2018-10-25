#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <outcome.hpp>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "ShaderModule.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "Fence.hpp"
#include "Semaphore.hpp"

namespace vka {

enum class PhysicalDeviceFeatures {
  robustBufferAccess,
  geometryShader,
  multiDrawIndirect,
  drawIndirectFirstInstance,
  fillModeNonSolid,
  multiViewport,
  samplerAnistropy
};

inline VkPhysicalDeviceFeatures makeVkFeatures(
    std::vector<PhysicalDeviceFeatures> features) {
  VkPhysicalDeviceFeatures vkFeatures{};
  for (auto feature : features) {
    switch (feature) {
      case PhysicalDeviceFeatures::robustBufferAccess:
        vkFeatures.robustBufferAccess = true;
        break;
      case PhysicalDeviceFeatures::geometryShader:
        vkFeatures.geometryShader = true;
        break;
      case PhysicalDeviceFeatures::multiDrawIndirect:
        vkFeatures.multiDrawIndirect = true;
        break;
      case PhysicalDeviceFeatures::drawIndirectFirstInstance:
        vkFeatures.drawIndirectFirstInstance = true;
        break;
      case PhysicalDeviceFeatures::fillModeNonSolid:
        vkFeatures.fillModeNonSolid = true;
        break;
      case PhysicalDeviceFeatures::multiViewport:
        vkFeatures.multiViewport = true;
        break;
      case PhysicalDeviceFeatures::samplerAnistropy:
        vkFeatures.samplerAnisotropy = true;
        break;
    }
  }
  return vkFeatures;
}

struct DeviceDeleter {
  using pointer = VkDevice;
  void operator()(VkDevice deviceHandle) {
    vkDestroyDevice(deviceHandle, nullptr);
  }
};
using DeviceOwner = std::unique_ptr<VkDevice, DeviceDeleter>;

struct AllocatorDeleter {
  using pointer = VmaAllocator;
  void operator()(VmaAllocator allocator) { vmaDestroyAllocator(allocator); }
};
using AllocatorOwner = std::unique_ptr<VmaAllocator, AllocatorDeleter>;

struct AllocatedBuffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocInfo;

  bool operator!=(std::nullptr_t) { return buffer != 0 || allocation != 0; }
  bool operator!=(const AllocatedBuffer& other) {
    return buffer != other.buffer || allocation != other.allocation;
  }
};

struct AllocatedBufferDeleter {
  using pointer = AllocatedBuffer;

  void operator()(AllocatedBuffer allocBuffer) {
    vmaDestroyBuffer(allocator, allocBuffer.buffer, allocBuffer.allocation);
  }

  VmaAllocator allocator;
};
using UniqueAllocatedBuffer =
    std::unique_ptr<AllocatedBuffer, AllocatedBufferDeleter>;

enum class ImageAspect {
  Color = VK_IMAGE_ASPECT_COLOR_BIT,
  Depth = VK_IMAGE_ASPECT_DEPTH_BIT
};

struct AllocatedImage {
  VkImage image;
  VmaAllocation allocation;
  VmaAllocationInfo allocInfo;

  bool operator!=(std::nullptr_t) { return image != 0 || allocation != 0; }
  bool operator!=(const AllocatedImage& other) {
    return image != other.image || allocation != other.allocation;
  }
};
struct AllocatedImageDeleter {
  using pointer = AllocatedImage;

  void operator()(AllocatedImage allocImage) {
    vmaDestroyImage(allocator, allocImage.image, allocImage.allocation);
  }
  VmaAllocator allocator;
};
using UniqueAllocatedImage =
    std::unique_ptr<AllocatedImage, AllocatedImageDeleter>;

struct ImageViewDeleter {
  using pointer = VkImageView;

  void operator()(VkImageView view) {
    vkDestroyImageView(device, view, nullptr);
  }

  VkDevice device;
};
using UniqueImageView = std::unique_ptr<VkImageView, ImageViewDeleter>;

struct FramebufferDeleter {
  using pointer = VkFramebuffer;

  void operator()(VkFramebuffer framebuffer) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }

  VkDevice device;
};
using UniqueFramebuffer = std::unique_ptr<VkFramebuffer, FramebufferDeleter>;
struct PhysicalDeviceData {
  PhysicalDeviceData() = default;
  PhysicalDeviceData(VkInstance);
  std::vector<VkPhysicalDevice> physicalDevices;
  std::map<VkPhysicalDevice, VkPhysicalDeviceProperties> properties;
  std::map<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties> memoryProperties;
  std::map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>>
      queueFamilyProperties;
};
using DeviceSelectCallback =
    std::function<VkPhysicalDevice(const PhysicalDeviceData&)>;
class Device {
public:
  Device() = delete;
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(
      VkInstance,
      VkSurfaceKHR,
      std::vector<const char*>,
      std::vector<PhysicalDeviceFeatures>,
      DeviceSelectCallback);
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;
  ~Device() = default;

  operator VkPhysicalDevice() { return physicalDeviceHandle; }
  operator VkDevice() { return deviceHandle; }
  uint32_t gfxQueueIndex() { return graphicsQueueIndex; }
  VmaAllocator getAllocator() { return allocator; }
  UniqueAllocatedBuffer
      createAllocatedBuffer(VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);
  UniqueAllocatedImage createAllocatedImage2D(
      VkExtent2D,
      VkFormat,
      VkImageUsageFlags,
      ImageAspect);
  UniqueImageView createImageView2D(VkImage, VkFormat, ImageAspect);
  std::unique_ptr<Swapchain> createSwapchain(
      VkSwapchainKHR = VK_NULL_HANDLE,
      VkFormat = VK_FORMAT_B8G8R8A8_UNORM);
  std::unique_ptr<RenderPass> createRenderPass(const VkRenderPassCreateInfo&);
  std::unique_ptr<PipelineCache> createPipelineCache();
  std::unique_ptr<PipelineCache> createPipelineCache(std::vector<char>);
  std::unique_ptr<GraphicsPipeline> createGraphicsPipeline(
      VkPipelineCache pipelineCache,
      const VkGraphicsPipelineCreateInfo&);
  std::unique_ptr<ComputePipeline> createComputePipeline(
      VkPipelineCache pipelineCache,
      const VkComputePipelineCreateInfo&);
  std::unique_ptr<CommandPool> createCommandPool();
  std::unique_ptr<DescriptorPool> createDescriptorPool(
      std::vector<VkDescriptorPoolSize> poolSizes,
      uint32_t maxSets);
  std::unique_ptr<DescriptorSetLayout> createSetLayout(
      std::vector<VkDescriptorSetLayoutBinding> bindings);
  std::unique_ptr<PipelineLayout> createPipelineLayout(
      std::vector<VkPushConstantRange>,
      std::vector<VkDescriptorSetLayout>);
  std::unique_ptr<ShaderModule> createShaderModule(std::string shaderPath);
  UniqueFramebuffer createFramebuffer(
      std::vector<VkImageView> attachments,
      VkRenderPass renderPass,
      uint32_t width,
      uint32_t height);
  std::unique_ptr<Fence> createFence(bool signaled = true);
  std::unique_ptr<Semaphore> createSemaphore();

  VkResult presentImage(
      VkSwapchainKHR swapchain,
      uint32_t imageIndex,
      VkSemaphore waitSemaphore);
  void queueSubmit(
      const std::vector<VkSemaphore>& waitSemaphores,
      const std::vector<VkCommandBuffer>& commandBuffers,
      const std::vector<VkSemaphore>& signalSemaphores,
      VkFence fence = VK_NULL_HANDLE);

  void waitIdle();

  VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
  void updateDescriptorSets(
      const std::vector<VkWriteDescriptorSet>& descriptorWrites);

  const VkPhysicalDeviceProperties& getDeviceProperties() {
    return deviceProperties;
  }

  const VkPhysicalDeviceMemoryProperties& getMemoryProperties() {
    return memoryProperties;
  }

private:
  VkSurfaceKHR surface;
  PhysicalDeviceData physicalDeviceData;
  VkPhysicalDevice physicalDeviceHandle;
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;

  VkDevice deviceHandle;
  DeviceOwner deviceOwner;
  VmaAllocator allocator;
  AllocatorOwner allocatorOwner;
  uint32_t graphicsQueueIndex = UINT32_MAX;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  VkQueue graphicsQueue;
};

}  // namespace vka