#pragma once

#include "VulkanFunctionLoader.hpp"
#include "vk_mem_alloc.h"
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "ShaderModule.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Pipeline.hpp"

namespace vka {

// A frame is tied to a set of resources:
// A command buffer is recorded, and for
//   rendering this requires one or more:
// Render passes, which can have multiple:
// Pipelines (and Shaders) which can render one or more:
// Materials, which can be applied to different:
// Meshes/Geometries, of which there can be multiple:
// Instances, each of which has its own transformation matrix

// steps to update
// 1. player input handling
// 2. physics update
// 3. world update (game logic)
// 4. sort instance data to optimal render order (least state changes)
//    (perhaps should store instances together so sorting isn't required)
// 5. stage data if not on unified memory
// 6. begin command buffer for copy
// 7. upload any data to device as needed
//    (transforms, other uniform data, etc.)
// 8. create pipeline barrier so that rendering occurs after data copy:
//    buffer barrier on the uniform buffers
//    source stage: transfer
//    dest stage: vertex shader
//    source access: transfer write
//    dest access: shader read
// 9. end command buffer

// steps to rendering a frame:
// 1. acquire swap image
// 2. acquire the most updated set of resources
// 3. begin command buffer recording
//    (should I use a command buffer per
//    world region as well as per frame?)

enum class PhysicalDeviceFeatures {
  robustBufferAccess,
  geometryShader,
  multiDrawIndirect,
  drawIndirectFirstInstance,
  fillModeNonSolid,
  multiViewport,
  samplerAnistropy
};

class Instance;
class CommandPool;
class CommandBuffer;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorSet;
class ShaderModule;
class GraphicsPipelineCreateInfo;
class ComputePipelineCreateInfo;
class GraphicsPipeline;
class ComputePipeline;

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

struct DeviceRequirements {
  std::vector<PhysicalDeviceFeatures> requiredFeatures;
  std::vector<const char*> deviceExtensions;
};
class Device {
  static constexpr uint32_t U32Max = ~(0ui32);

public:
  Device() = delete;
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(VkInstance, DeviceRequirements);
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;
  ~Device() = default;
  operator VkDevice() { return deviceHandle; }
  uint32_t gfxQueueIndex() { return graphicsQueueIndex; }
  VmaAllocator getAllocator() { return allocator; }
  GraphicsPipeline* createGrapicsPipeline(const VkGraphicsPipelineCreateInfo&);
  ComputePipeline* createComputePipeline(const VkComputePipelineCreateInfo&);
  CommandPool* createCommandPool();
  DescriptorPool* createDescriptorPool();
  DescriptorSetLayout* createSetLayout(
      std::vector<VkDescriptorSetLayoutBinding> bindings);
  ShaderModule* createShaderModule(std::string shaderPath);

private:
  VkInstance instance;
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
  uint32_t graphicsQueueIndex = U32Max;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  VkQueue graphicsQueue;

  std::vector<std::unique_ptr<GraphicsPipeline>> graphicsPipelines;
  std::vector<std::unique_ptr<ComputePipeline>> computePipelines;
  std::vector<std::unique_ptr<CommandPool>> commandPools;
  std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;
  std::vector<std::unique_ptr<DescriptorSetLayout>> descriptorSetLayouts;
  std::vector<std::unique_ptr<ShaderModule>> shaderModules;
};

}  // namespace vka