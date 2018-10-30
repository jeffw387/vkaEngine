#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

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
      VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
      VkImageType imageType = VK_IMAGE_TYPE_2D,
      uint32_t mipLevels = 1,
      uint32_t arrayLayers = 1,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
      VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);
  Image(VkImage);
  ~Image();
  operator VkImage() const noexcept;
  operator VmaAllocation() const noexcept;

private:
  VmaAllocator allocator = nullptr;
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

private:
  VkDevice device;
  VkImageView view;
};

class Sampler {
public:
  Sampler(
      VkDevice,
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
  ~Sampler();
  operator VkSampler() const noexcept;

private:
  VkDevice device = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;
};
}  // namespace vka