#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include "image.hpp"
#include "move_into.hpp"

namespace vka {
struct image_view {
  explicit image_view(
      VkDevice device,
      VkImageView imageView)
      : m_device(device), m_imageView(imageView) {}

  image_view(const image_view&) = delete;
  image_view(image_view&&) = default;
  image_view& operator=(const image_view&) = delete;
  image_view& operator=(image_view&&) = default;

  ~image_view() noexcept {
    vkDestroyImageView(m_device, m_imageView, nullptr);
  }

  operator VkImageView() const noexcept {
    return m_imageView;
  }

private:
  VkDevice m_device = {};
  VkImageView m_imageView = {};
};

struct image_type_not_supported {};

inline auto convert = [](VkImageType type, bool arrayType) {
  switch (type) {
    case VK_IMAGE_TYPE_1D:
      return arrayType ? VK_IMAGE_VIEW_TYPE_1D_ARRAY
                       : VK_IMAGE_VIEW_TYPE_1D;
    case VK_IMAGE_TYPE_2D:
      return arrayType ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                       : VK_IMAGE_VIEW_TYPE_2D;
    case VK_IMAGE_TYPE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
    default:
      return VK_IMAGE_VIEW_TYPE_1D;
  }
};

using image_view_expected =
    tl::expected<std::unique_ptr<image_view>, VkResult>;
struct image_view_builder {
  image_view_expected build(VkDevice device) {
    image_view_expected expectedView = {};
    VkImageViewCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    createInfo.format = m_format;
    createInfo.image = m_image;
    createInfo.viewType =
        convert(m_imageType, m_arrayLayers > 1);
    createInfo.subresourceRange = m_subrange;

    VkImageView imageView = {};
    auto result = vkCreateImageView(
        device, &createInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<image_view>(device, imageView);
  }

  image_view_builder& from_image(const image& sourceImage) {
    m_image = sourceImage;
    m_arrayLayers = sourceImage.array_layers();
    m_imageType = sourceImage.image_type();
    m_format = sourceImage.image_format();
    m_subrange.aspectMask = static_cast<VkImageAspectFlags>(
        sourceImage.get_image_aspect());
    m_subrange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    m_subrange.levelCount = VK_REMAINING_MIP_LEVELS;
    return *this;
  }

  image_view_builder& image_source(VkImage imageSource) {
    m_image = imageSource;
    return *this;
  }

  image_view_builder& image_format(VkFormat format) {
    m_format = format;
    return *this;
  }

  image_view_builder& array_layers(uint32_t count) {
    m_arrayLayers = count;
    return *this;
  }

  image_view_builder& image_type(VkImageType type) {
    m_imageType = type;
    return *this;
  }

  image_view_builder& image_aspect(
      VkImageAspectFlags aspect) {
    m_subrange.aspectMask = aspect;
    m_subrange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    m_subrange.levelCount = VK_REMAINING_MIP_LEVELS;
    return *this;
  }

private:
  VkImage m_image = {};
  VkImageType m_imageType = {};
  uint32_t m_arrayLayers = {};
  VkFormat m_format = {};
  VkImageSubresourceRange m_subrange = {};
};
}  // namespace vka