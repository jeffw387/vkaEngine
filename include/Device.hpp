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
#include "Framebuffer.hpp"
#include "Image.hpp"
#include "Buffer.hpp"

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
  ~Device();

  operator VkPhysicalDevice() { return physicalDevice; }
  operator VkDevice() { return device; }
  uint32_t gfxQueueIndex() { return graphicsQueueIndex; }
  VmaAllocator getAllocator() { return allocator; }
  std::shared_ptr<Buffer> createBuffer(
      VkDeviceSize,
      VkBufferUsageFlags,
      VmaMemoryUsage,
      bool dedicated = false);
  std::shared_ptr<Image> createImage2D(
      VkExtent2D,
      VkFormat,
      VkImageUsageFlags,
      ImageAspect,
      bool = false);
  std::unique_ptr<ImageView> createImageView2D(VkImage, VkFormat, ImageAspect);
  std::unique_ptr<Swapchain> createSwapchain(
      VkSwapchainKHR = VK_NULL_HANDLE,
      VkFormat = VK_FORMAT_B8G8R8A8_UNORM);
  std::shared_ptr<RenderPass> createRenderPass(const VkRenderPassCreateInfo&);
  std::unique_ptr<PipelineCache> createPipelineCache();
  std::unique_ptr<PipelineCache> createPipelineCache(std::vector<char>);
  std::shared_ptr<GraphicsPipeline> createGraphicsPipeline(
      VkPipelineCache pipelineCache,
      const VkGraphicsPipelineCreateInfo&);
  std::shared_ptr<ComputePipeline> createComputePipeline(
      VkPipelineCache pipelineCache,
      const VkComputePipelineCreateInfo&);
  std::unique_ptr<CommandPool> createCommandPool(
      bool primary = true,
      bool transient = true);
  std::unique_ptr<DescriptorPool> createDescriptorPool(
      std::vector<VkDescriptorPoolSize> poolSizes,
      uint32_t maxSets);
  std::unique_ptr<DescriptorSetLayout> createSetLayout(
      std::vector<VkDescriptorSetLayoutBinding> bindings);
  std::shared_ptr<PipelineLayout> createPipelineLayout(
      std::vector<VkPushConstantRange>,
      std::vector<VkDescriptorSetLayout>);
  std::unique_ptr<ShaderModule> createShaderModule(std::string shaderPath);
  std::unique_ptr<Sampler> createSampler(
      VkFilter magFilter = VK_FILTER_NEAREST,
      VkFilter minFilter = VK_FILTER_NEAREST,
      VkSamplerMipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
      VkSamplerAddressMode U = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VkSamplerAddressMode V = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VkSamplerAddressMode W = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      float mipLodBias = 0.f,
      VkBool32 anisotropyEnable = false,
      float maxAnisotropy = 0.f,
      VkBool32 compareEnable = false,
      VkCompareOp compareOp = VK_COMPARE_OP_NEVER,
      float minLod = 0.f,
      float maxLod = 0.f,
      VkBorderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
      VkBool32 unnormalizedCoordinates = false);
  std::shared_ptr<Framebuffer> createFramebuffer(
      VkRenderPass renderPass,
      std::vector<std::shared_ptr<ImageView>> attachments,
      VkExtent2D extent);
  std::unique_ptr<Fence> createFence(bool signaled = true);
  std::unique_ptr<Semaphore> createSemaphore();

  VkResult presentImage(
      VkSwapchainKHR swapchain,
      uint32_t imageIndex,
      VkSemaphore waitSemaphore);
  void queueSubmit(
      const std::vector<VkSemaphore>& waitSemaphores,
      std::vector<std::shared_ptr<CommandBuffer>> commandBuffers,
      const std::vector<VkSemaphore>& signalSemaphores,
      vka::Fence* fence = nullptr);

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

  template <typename T>
  void debugNameObject(VkObjectType objectType, T handle, const char* name) {
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = reinterpret_cast<uint64_t>(handle);
    nameInfo.pObjectName = name;
    Device::vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
  }

private:
  VkSurfaceKHR surface;
  PhysicalDeviceData physicalDeviceData;
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;

  VkDevice device;
  VmaAllocator allocator;
  uint32_t graphicsQueueIndex = UINT32_MAX;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  VkQueue graphicsQueue;
  static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
};

}  // namespace vka