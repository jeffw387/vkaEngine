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
}  // namespace vka