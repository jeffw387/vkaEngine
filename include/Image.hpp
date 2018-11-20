#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include "thsvs_simpler_vulkan_synchronization.h"

namespace vka {

enum class ImageAspect {
  Color = VK_IMAGE_ASPECT_COLOR_BIT,
  Depth = VK_IMAGE_ASPECT_DEPTH_BIT
};

class Image {
public:
  Image(
      VmaAllocator allocator,
      std::vector<uint32_t> queueIndices,
      VkExtent3D extent,
      VkFormat format,
      VkImageUsageFlags usage,
      ImageAspect aspect,
      bool dedicated,
      VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
      VkImageType imageType = VK_IMAGE_TYPE_2D,
      uint32_t arrayLayers = 1,
      uint32_t mipLevels = 1,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
      ThsvsImageLayout initialLayout = THSVS_IMAGE_LAYOUT_OPTIMAL,
      VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);
  Image(VkImage);
  ~Image();
  operator VkImage() const noexcept;
  operator VmaAllocation() const noexcept;
  void cmdExecuted() {}

private:
  VmaAllocator allocator = nullptr;

public:
  VkExtent3D extent = {};
  VkFormat format = {};
  ImageAspect aspect = {};
  uint32_t layerCount = 1;
  ThsvsImageLayout layout = {};

private:
  VkImage image = VK_NULL_HANDLE;
  VmaAllocation allocation = nullptr;
};

class ImageView {
public:
  ImageView(
      VkDevice device,
      VkImage image,
      VkFormat format,
      ImageAspect aspect,
      VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
  ~ImageView();
  operator VkImageView() const noexcept;
  void cmdExecuted() {}

private:
  VkDevice device;
  VkImageView view;
};

class Sampler {
public:
  Sampler(
      VkDevice,
      VkFilter magFilter,
      VkFilter minFilter,
      VkSamplerMipmapMode,
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
      VkBorderColor,
      VkBool32 unnormalizedCoordinates);
  ~Sampler();
  operator VkSampler() const noexcept;
  operator const VkSampler*() const noexcept;

private:
  VkDevice device = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;
};
}  // namespace vka