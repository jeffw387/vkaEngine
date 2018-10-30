#include "Image.hpp"
#include <vk_mem_alloc.h>

namespace vka {
Image::Image(
    VmaAllocator allocator,
    std::vector<uint32_t> queueIndices,
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    ImageAspect aspect,
    VkSampleCountFlagBits samples,
    VkImageType imageType,
    uint32_t mipLevels,
    uint32_t arrayLayers,
    VmaMemoryUsage memoryUsage,
    VkImageLayout initialLayout,
    VkImageTiling tiling)
    : allocator(allocator) {
  VkImageCreateInfo imageCreateInfo{};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.usage = usage;
  imageCreateInfo.format = format;
  imageCreateInfo.extent = extent;
  imageCreateInfo.mipLevels = mipLevels;
  imageCreateInfo.arrayLayers = arrayLayers;
  imageCreateInfo.initialLayout = initialLayout;
  imageCreateInfo.pQueueFamilyIndices = queueIndices.data();
  imageCreateInfo.queueFamilyIndexCount =
      static_cast<uint32_t>(queueIndices.size());
  imageCreateInfo.imageType = imageType;
  imageCreateInfo.tiling = tiling;
  imageCreateInfo.samples = samples;
  imageCreateInfo.sharingMode = queueIndices.size() > 1
                                    ? VK_SHARING_MODE_CONCURRENT
                                    : VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = memoryUsage;

  vmaCreateImage(
      allocator,
      &imageCreateInfo,
      &allocationCreateInfo,
      &image,
      &allocation,
      nullptr);
}

Image::operator VkImage() const noexcept { return image; }

Image::operator VmaAllocation() const noexcept { return allocation; }

Image::~Image() {
  if (allocator != nullptr && image != VK_NULL_HANDLE &&
      allocation != nullptr) {
    vmaDestroyImage(allocator, image, allocation);
  }
}

ImageView::ImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    ImageAspect aspect,
    VkImageViewType viewType)
    : device(device) {
  VkImageViewCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.format = format;
  createInfo.image = image;
  createInfo.viewType = viewType;
  VkImageSubresourceRange range{};
  range.aspectMask = static_cast<VkImageAspectFlagBits>(aspect);
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;
  range.levelCount = VK_REMAINING_MIP_LEVELS;
  createInfo.subresourceRange = std::move(range);
  vkCreateImageView(device, &createInfo, nullptr, &view);
}

ImageView::~ImageView() {
  if (device != VK_NULL_HANDLE && view != VK_NULL_HANDLE) {
    vkDestroyImageView(device, view, nullptr);
  }
}

ImageView::operator VkImageView() const noexcept { return view; }

Sampler::Sampler(
    VkDevice device,
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
    VkBool32 unnormalizedCoordinates)
    : device(device) {
  VkSamplerCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.magFilter = magFilter;
  createInfo.minFilter = minFilter;
  createInfo.mipmapMode = mipmapMode;
  createInfo.addressModeU = U;
  createInfo.addressModeV = V;
  createInfo.addressModeW = W;
  createInfo.mipLodBias = mipLodBias;
  createInfo.anisotropyEnable = anisotropyEnable;
  createInfo.maxAnisotropy = maxAnisotropy;
  createInfo.compareEnable = compareEnable;
  createInfo.compareOp = compareOp;
  createInfo.minLod = minLod;
  createInfo.maxLod = maxLod;
  createInfo.borderColor = borderColor;
  createInfo.unnormalizedCoordinates = unnormalizedCoordinates;

  vkCreateSampler(device, &createInfo, nullptr, &sampler);
}

Sampler::~Sampler() {
  if (device != VK_NULL_HANDLE && sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, sampler, nullptr);
  }
}

Sampler::operator VkSampler() const noexcept { return sampler; }
}  // namespace vka