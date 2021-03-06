#pragma once

#include <vulkan/vulkan.h>
#include <algorithm>
#include <memory>
#include <tl/expected.hpp>
#include <vector>

namespace vka {
struct swapchain {
  explicit swapchain(
      VkDevice device,
      VkSwapchainKHR swapchain)
      : m_device(device), m_swapchain(swapchain) {}
  ~swapchain() {
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  }
  swapchain(const swapchain&) = delete;
  swapchain(swapchain&&) = default;
  swapchain& operator=(const swapchain&) = delete;
  swapchain& operator=(swapchain&&) = default;
  operator VkSwapchainKHR() const noexcept {
    return m_swapchain;
  }

private:
  VkDevice m_device = {};
  VkSwapchainKHR m_swapchain = {};
};

auto presentModeValidate =
    [](VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface,
       VkPresentModeKHR present)
    -> tl::expected<VkPresentModeKHR, VkResult> {
  uint32_t count = {};
  auto countResult =
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &count, nullptr);
  if (countResult != VK_SUCCESS) {
    return tl::make_unexpected(countResult);
  }
  std::vector<VkPresentModeKHR> modes = {};
  modes.resize(count);
  auto enumerateResult =
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &count, modes.data());
  if (enumerateResult != VK_SUCCESS) {
    return tl::make_unexpected(enumerateResult);
  }

  if (std::find(
          std::begin(modes), std::end(modes), present) !=
      std::end(modes)) {
    return present;
  }
  return tl::make_unexpected(
      VkResult::VK_ERROR_FEATURE_NOT_PRESENT);
};

auto surfaceFormatSelect =
    [](VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface,
       VkFormat format,
       VkColorSpaceKHR colorSpace)
    -> tl::expected<VkSurfaceFormatKHR, VkResult> {
  uint32_t count = {};
  auto countResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, surface, &count, nullptr);
  if (countResult != VK_SUCCESS) {
    return tl::make_unexpected(countResult);
  }
  std::vector<VkSurfaceFormatKHR> formats = {};
  formats.resize(count);
  auto enumerateResult =
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          physicalDevice, surface, &count, formats.data());
  if (enumerateResult != VK_SUCCESS) {
    return tl::make_unexpected(enumerateResult);
  }

  auto preferred = VkSurfaceFormatKHR{format, colorSpace};
  if (std::find_if(
          std::begin(formats),
          std::end(formats),
          [=](VkSurfaceFormatKHR testFormat) {
            return (testFormat.format ==
                    preferred.format) &&
                   (testFormat.colorSpace ==
                    preferred.colorSpace);
          }) != std::end(formats)) {
    return preferred;
  }
  if (count > 0) {
    return formats[0];
  }
  return tl::make_unexpected(
      VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED);
};

auto imageExtentSelect = [](VkPhysicalDevice physicalDevice,
                            VkSurfaceKHR surface)
    -> tl::expected<VkExtent2D, VkResult> {
  VkSurfaceCapabilitiesKHR capabilities = {};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &capabilities);
  if (capabilities.currentExtent.width == 0 ||
      capabilities.currentExtent.height == 0) {
    return tl::make_unexpected(VkResult::VK_NOT_READY);
  }
  return capabilities.currentExtent;
};

auto transformSelect = [](VkPhysicalDevice physicalDevice,
                          VkSurfaceKHR surface) {
  VkSurfaceCapabilitiesKHR capabilities = {};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &capabilities);
  return capabilities.currentTransform;
};

auto compositeAlphaSelect =
    [](VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface) {
      return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    };

struct swapchain_builder {
  tl::expected<std::unique_ptr<swapchain>, VkResult> build(
      VkPhysicalDevice physicalDevice,
      VkSurfaceKHR surface,
      VkDevice device) {
    presentModeValidate(
        physicalDevice, surface, m_presentMode)
        .map([& destination = m_createInfo.presentMode](
                 auto presentMode) {
          destination = presentMode;
        })
        .map_error([](auto error) { return error; });
    surfaceFormatSelect(
        physicalDevice, surface, m_format, m_colorSpace)
        .map([& format = m_createInfo.imageFormat,
              &colorSpace = m_createInfo.imageColorSpace](
                 auto surfaceFormat) {
          format = surfaceFormat.format;
          colorSpace = surfaceFormat.colorSpace;
        })
        .map_error([](auto error) { return error; });
    imageExtentSelect(physicalDevice, surface)
        .map([& extent = m_createInfo.imageExtent](
                 auto value) { extent = value; })
        .map_error([](auto error) { return error; });
    m_createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    m_createInfo.preTransform =
        transformSelect(physicalDevice, surface);
    m_createInfo.compositeAlpha =
        compositeAlphaSelect(physicalDevice, surface);
    m_createInfo.imageArrayLayers = m_imageArrayLayers;
    m_createInfo.minImageCount = m_imageCount;
    m_createInfo.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE;
    m_createInfo.pQueueFamilyIndices = &m_queueFamilyIndex;
    m_createInfo.queueFamilyIndexCount = 1U;
    m_createInfo.surface = surface;
    VkSwapchainKHR swapchain = {};
    auto result = vkCreateSwapchainKHR(
        device, &m_createInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<vka::swapchain>(
        device, swapchain);
  }

  swapchain_builder& present_mode(
      VkPresentModeKHR presentMode) {
    m_presentMode = presentMode;
    return *this;
  }

  swapchain_builder& image_count(uint32_t imageCount) {
    m_imageCount = imageCount;
    return *this;
  }

  swapchain_builder& queue_family_index(
      uint32_t queueFamilyIndex) {
    m_queueFamilyIndex = queueFamilyIndex;
    return *this;
  }

  swapchain_builder& image_format(VkFormat format) {
    m_format = format;
    return *this;
  }

  swapchain_builder& image_color_space(
      VkColorSpaceKHR colorSpace) {
    m_colorSpace = colorSpace;
    return *this;
  }

private:
  uint32_t m_imageCount = 3U;
  uint32_t m_imageArrayLayers = 1U;
  VkPresentModeKHR m_presentMode = {};
  VkFormat m_format = VK_FORMAT_B8G8R8A8_UNORM;
  VkColorSpaceKHR m_colorSpace =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  uint32_t m_queueFamilyIndex = {};
  VkSwapchainCreateInfoKHR m_createInfo = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
};
}  // namespace vka