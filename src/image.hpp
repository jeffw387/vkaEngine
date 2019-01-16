#pragma once

#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <vk_mem_alloc.h>
#include <memory>

namespace vka {
enum class image_aspect {
  color = VK_IMAGE_ASPECT_COLOR_BIT,
  depth = VK_IMAGE_ASPECT_DEPTH_BIT
};

struct image {
  explicit image(
      VmaAllocator allocator,
      VmaAllocation allocation,
      VkImage imageHandle,
      VkImageType imageType,
      VkFormat imageFormat,
      uint32_t arrayLayers,
      image_aspect aspect)
      : m_allocator(allocator),
        m_allocation(allocation),
        m_image(imageHandle),
        m_imageType(imageType),
        m_imageFormat(imageFormat),
        m_arrayLayers(arrayLayers),
        m_aspect(aspect) {}

  image(const image&) = delete;
  image(image&&) = default;
  image& operator=(const image&) = delete;
  image& operator=(image&&) = default;

  ~image() noexcept { vmaDestroyImage(m_allocator, m_image, m_allocation); }

  operator VkImage() const noexcept { return m_image; }
  operator VmaAllocation() const noexcept { return m_allocation; }

  VkImageType image_type() const noexcept { return m_imageType; }

  VkFormat image_format() const noexcept { return m_imageFormat; }

  uint32_t array_layers() const noexcept { return m_arrayLayers; }

  image_aspect get_image_aspect() const noexcept { return m_aspect; }

private:
  VmaAllocator m_allocator = {};
  VmaAllocation m_allocation = {};
  VkImage m_image = {};
  VkImageType m_imageType = {};
  VkFormat m_imageFormat = {};
  uint32_t m_arrayLayers = {};
  image_aspect m_aspect = {};
};

struct image_builder {
  tl::expected<std::unique_ptr<image>, VkResult> build(VmaAllocator allocator) {
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = m_allocationFlags;
    allocationCreateInfo.usage = m_memoryUsage;
    allocationCreateInfo.pool = m_memoryPool;

    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.format = m_format;
    imageCreateInfo.imageType = m_imageType;
    imageCreateInfo.tiling = m_tiling;
    imageCreateInfo.samples = m_samples;
    imageCreateInfo.mipLevels = m_mipLevels;
    imageCreateInfo.usage = m_imageUsage;
    imageCreateInfo.initialLayout =
        ((m_tiling == VK_IMAGE_TILING_OPTIMAL)
             ? VK_IMAGE_LAYOUT_UNDEFINED
             : VK_IMAGE_LAYOUT_PREINITIALIZED);
    imageCreateInfo.queueFamilyIndexCount = 1;
    imageCreateInfo.pQueueFamilyIndices = &m_queueFamilyIndex;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.arrayLayers = m_arrayLayers;
    imageCreateInfo.extent = m_imageExtent;

    VmaAllocation allocation = {};
    VkImage imageHandle = {};
    auto result = vmaCreateImage(
        allocator,
        &imageCreateInfo,
        &allocationCreateInfo,
        &imageHandle,
        &allocation,
        nullptr);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<image>(
        allocator,
        allocation,
        imageHandle,
        m_imageType,
        m_format,
        m_arrayLayers,
        m_aspect);
  }

  image_builder& format(VkFormat imageFormat) {
    m_format = imageFormat;
    return *this;
  }

  image_builder& type_1d() {
    m_imageType = VK_IMAGE_TYPE_1D;
    return *this;
  }
  image_builder& type_2d() {
    m_imageType = VK_IMAGE_TYPE_2D;
    return *this;
  }
  image_builder& type_3d() {
    m_imageType = VK_IMAGE_TYPE_3D;
    return *this;
  }

  image_builder&
  image_extent(uint32_t width = 1, uint32_t height = 1, uint32_t depth = 1) {
    m_imageExtent = {width, height, depth};
    return *this;
  }

  image_builder& array_layers(uint32_t count) {
    m_arrayLayers = count;
    return *this;
  }

  image_builder& mip_levels(uint32_t count) {
    m_mipLevels = count;
    return *this;
  }

  // NOTE: can be implied from usage perhaps
  // image_builder& aspect_color() {}
  // image_builder& aspect_depth() {}

  image_builder& samples_1() {
    m_samples = VK_SAMPLE_COUNT_1_BIT;
    return *this;
  }
  image_builder& samples_2() {
    m_samples = VK_SAMPLE_COUNT_2_BIT;
    return *this;
  }
  image_builder& samples_4() {
    m_samples = VK_SAMPLE_COUNT_4_BIT;
    return *this;
  }
  image_builder& samples_8() {
    m_samples = VK_SAMPLE_COUNT_8_BIT;
    return *this;
  }
  image_builder& samples_16() {
    m_samples = VK_SAMPLE_COUNT_16_BIT;
    return *this;
  }
  image_builder& samples_32() {
    m_samples = VK_SAMPLE_COUNT_32_BIT;
    return *this;
  }
  image_builder& samples_64() {
    m_samples = VK_SAMPLE_COUNT_64_BIT;
    return *this;
  }

  image_builder& optimal_tiling() {
    m_tiling = VK_IMAGE_TILING_OPTIMAL;
    return *this;
  }
  image_builder& linear_tiling() {
    m_tiling = VK_IMAGE_TILING_LINEAR;
    return *this;
  }

  image_builder& transfer_source() {
    m_imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    return *this;
  }
  image_builder& transfer_destination() {
    m_imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return *this;
  }
  image_builder& sampled() {
    m_imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    return *this;
  }
  image_builder& color_attachment() {
    m_imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    m_aspect = image_aspect::color;
    return *this;
  }
  image_builder& depth_attachment() {
    m_imageUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    m_aspect = image_aspect::depth;
    return *this;
  }
  image_builder& transient() {
    m_imageUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    return *this;
  }
  image_builder& input_attachment() {
    m_imageUsage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    return *this;
  }

  image_builder& dedicated() {
    m_allocationFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    return *this;
  }

  image_builder& cpu_only() {
    m_memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    return *this;
  }

  image_builder& gpu_only() {
    m_memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    return *this;
  }

  image_builder& cpu_to_gpu() {
    m_memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    return *this;
  }

  image_builder& gpu_to_cpu() {
    m_memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    return *this;
  }

  image_builder& queue_family_index(uint32_t index) {
    m_queueFamilyIndex = index;
    return *this;
  }

  image_builder& memory_pool(VmaPool memoryPool) {
    m_memoryPool = memoryPool;
    return *this;
  }

private:
  VkFormat m_format = {};
  VkImageType m_imageType = {};
  VkExtent3D m_imageExtent = {};
  VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
  uint32_t m_arrayLayers = 1;
  uint32_t m_mipLevels = 1;
  VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
  VkImageUsageFlags m_imageUsage = {};
  VmaAllocationCreateFlags m_allocationFlags = {};
  VmaMemoryUsage m_memoryUsage = {};
  VmaPool m_memoryPool = {};
  uint32_t m_queueFamilyIndex = {};
  image_aspect m_aspect = {};
};
}  // namespace vka