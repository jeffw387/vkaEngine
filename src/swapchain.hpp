#include <vulkan/vulkan.h>
#include <memory>
#include <algorithm>
#include <expected.hpp>

namespace vka {
struct swapchain {
  explicit swapchain(VkDevice device, VkSwapchainKHR swapchain)
      : m_device(device), m_swapchain(swapchain) {}

private:
  VkDevice m_device = {};
  VkSwapchainKHR m_swapchain = {};
};

auto presentModeValidate =
    [](VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface,
       VkPresentModeKHR present) -> tl::expected<VkPresentModeKHR, VkResult> {
  uint32_t count = {};
  auto countResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &count, nullptr);
  if (countResult != VK_SUCCESS) {
    return tl::make_unexpected(countResult);
  }
  std::vector<VkPresentModeKHR> modes = {};
  modes.resize(count);
  auto enumerateResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &count, modes.data());
  if (enumerateResult != VK_SUCCESS) {
    return tl::make_unexpected(enumerateResult);
  }

  if (std::find(std::begin(modes), std::end(modes), present) !=
      std::end(modes)) {
    return present;
  }
};

auto surfaceFormatSelect = 
    [](VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface,
       VkFormat format,
       VkColorSpaceKHR colorSpace) -> tl::expected<VkSurfaceFormatKHR, VkResult> {
         uint32_t count = {};
         auto countResult = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
         if (countResult != VK_SUCCESS) {
           return tl::make_unexpected(countResult);
         }
         std::vector<VkSurfaceFormatKHR> formats = {};
         formats.resize(count);
         auto enumerateResult = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
         if (enumerateResult != VK_SUCCESS) {
           return tl::make_unexpected(enumerateResult);
         }


       }

struct swapchain_builder {
  tl::expected<std::unique_ptr<swapchain>, VkResult> build(
      VkPhysicalDevice physicalDevice,
      VkSurfaceKHR surface,
      VkDevice device) {
    VkSwapchainKHR swapchain = {};
    auto result = vkCreateSwapchainKHR(device, &m_createInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
  }

  swapchain_builder& present_mode(VkPresentModeKHR presentMode) {
    m_presentMode = presentMode;
    return *this;
  }

  swapchain_builder& image_count(uint32_t imageCount) {
    m_imageCount = imageCount;
    return *this;
  }

  swapchain_builder& queue_family_index(uint32_t queueFamilyIndex) {
    m_queueFamilyIndex = queueFamilyIndex;
    return *this;
  }

  swapchain_builder& image_format(VkFormat format) {
    m_format = format;
    return *this;
  }


private:
  uint32_t m_imageCount = 3U;
  VkPresentModeKHR m_presentMode = {};
  uint32_t m_queueFamilyIndex = {};
  VkSwapchainCreateInfoKHR m_createInfo = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
};
}  // namespace vka