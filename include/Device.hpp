#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "ShaderModule.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "outcome.hpp"

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

  AllocatedBufferDeleter() = default;
  AllocatedBufferDeleter(std::nullptr_t) : allocator(0){};
  AllocatedBufferDeleter(VmaAllocator allocator) : allocator(allocator) {}

  void operator()(AllocatedBuffer allocBuffer) {
    vmaDestroyBuffer(allocator, allocBuffer.buffer, allocBuffer.allocation);
  }

  VmaAllocator allocator;
};
using UniqueAllocatedBuffer =
    std::unique_ptr<AllocatedBuffer, AllocatedBufferDeleter>;

struct DeviceRequirements {
  std::vector<PhysicalDeviceFeatures> requiredFeatures;
  std::vector<const char*> deviceExtensions;
};
class Device {
public:
  Device() = delete;
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(VkInstance, VkSurfaceKHR, DeviceRequirements);
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;
  ~Device() = default;

  operator VkDevice() { return deviceHandle; }
  uint32_t gfxQueueIndex() { return graphicsQueueIndex; }
  VmaAllocator getAllocator() { return allocator; }
  AllocatedBuffer createAllocatedBuffer(
      VkDeviceSize,
      VkBufferUsageFlags,
      VmaAllocationCreateFlags,
      VmaMemoryUsage);
  Swapchain* createSwapchain();
  RenderPass* createRenderPass(const VkRenderPassCreateInfo&);
  GraphicsPipeline* createGraphicsPipeline(const VkGraphicsPipelineCreateInfo&);
  ComputePipeline* createComputePipeline(const VkComputePipelineCreateInfo&);
  CommandPool* createCommandPool();
  DescriptorPool* createDescriptorPool(
      const std::vector<VkDescriptorPoolSize>& poolSizes,
      uint32_t maxSets);
  DescriptorSetLayout* createSetLayout(
      const std::vector<VkDescriptorSetLayoutBinding>& bindings);
  PipelineLayout* createPipelineLayout(
      const std::vector<VkPushConstantRange>&,
      const std::vector<VkDescriptorSetLayout>&);
  ShaderModule* createShaderModule(std::string shaderPath);

  VkResult presentImage(uint32_t imageIndex, VkSemaphore waitSemaphore);
  void queueSubmit(
      const std::vector<VkSemaphore>& waitSemaphores,
      const std::vector<VkCommandBuffer>& commandBuffers,
      const std::vector<VkSemaphore>& signalSemaphores);

  void waitIdle();

  VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
  void updateDescriptorSets(
      const std::vector<VkWriteDescriptorSet>& descriptorWrites);

private:
  VkSurfaceKHR surface;
  DeviceRequirements requirements;
  std::shared_ptr<spdlog::logger> multilogger;
  std::vector<VkPhysicalDevice> physicalDevices;
  VkPhysicalDevice physicalDeviceHandle;
  VkPhysicalDeviceProperties deviceProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  VkDevice deviceHandle;
  DeviceOwner deviceOwner;
  VmaAllocator allocator;
  AllocatorOwner allocatorOwner;
  uint32_t graphicsQueueIndex = UINT32_MAX;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  VkQueue graphicsQueue;

  std::unique_ptr<Swapchain> swapchain;
  std::unique_ptr<PipelineCache> pipelineCache;
  std::vector<UniqueAllocatedBuffer> allocatedBuffers;
  std::vector<std::unique_ptr<RenderPass>> renderPasses;
  std::vector<std::unique_ptr<PipelineLayout>> pipelineLayouts;
  std::vector<std::unique_ptr<GraphicsPipeline>> graphicsPipelines;
  std::vector<std::unique_ptr<ComputePipeline>> computePipelines;
  std::vector<std::unique_ptr<CommandPool>> commandPools;
  std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
  std::vector<std::unique_ptr<DescriptorSetLayout>> descriptorSetLayouts;
  std::vector<std::unique_ptr<ShaderModule>> shaderModules;
};

}  // namespace vka