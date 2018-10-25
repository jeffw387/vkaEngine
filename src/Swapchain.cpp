#include "Swapchain.hpp"
#include "Engine.hpp"

namespace vka {
SwapchainCreateInfo::SwapchainCreateInfo() {
  createInfo = VkSwapchainCreateInfoKHR{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.minImageCount = 3;
  createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  createInfo.imageExtent = {1, 1};
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  createInfo.clipped = false;
}

void SwapchainCreateInfo::setSurface(VkSurfaceKHR surface) {
  createInfo.surface = surface;
}
void SwapchainCreateInfo::setMinImageCount(uint32_t minImageCount) {
  createInfo.minImageCount = minImageCount;
}
void SwapchainCreateInfo::setImageFormat(VkFormat imageFormat) {
  createInfo.imageFormat = imageFormat;
}
void SwapchainCreateInfo::setImageColorSpace(VkColorSpaceKHR imageColorSpace) {
  createInfo.imageColorSpace = imageColorSpace;
}
void SwapchainCreateInfo::setImageExtent(VkExtent2D imageExtent) {
  createInfo.imageExtent = imageExtent;
}
void SwapchainCreateInfo::addQueueFamilyIndex(uint32_t queueFamilyIndex) {
  queueFamilyIndices.push_back(queueFamilyIndex);
}
void SwapchainCreateInfo::setSurfacePreTransform(
    VkSurfaceTransformFlagBitsKHR preTransform) {
  createInfo.preTransform = preTransform;
}
void SwapchainCreateInfo::setPresentMode(VkPresentModeKHR presentMode) {
  createInfo.presentMode = presentMode;
}

void SwapchainCreateInfo::setOldSwapchain(VkSwapchainKHR oldSwapchain) {
  createInfo.oldSwapchain = oldSwapchain;
}

SwapchainCreateInfo::operator const VkSwapchainCreateInfoKHR&() {
  createInfo.queueFamilyIndexCount =
      static_cast<uint32_t>(queueFamilyIndices.size());
  createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  return createInfo;
}

Swapchain::Swapchain(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t graphicsQueueIndex,
    const VkSwapchainCreateInfoKHR& createInfo)
    : device(device), swapExtent(createInfo.imageExtent) {
  VkBool32 presentSupport{};
  vkGetPhysicalDeviceSurfaceSupportKHR(
      physicalDevice, graphicsQueueIndex, createInfo.surface, &presentSupport);
  MultiLogger::get()->info("Surface present support: {}", presentSupport);
  uint32_t formatCount{};
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, createInfo.surface, &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  surfaceFormats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, createInfo.surface, &formatCount, surfaceFormats.data());

  for (const auto& surfFormat : surfaceFormats) {
    MultiLogger::get()->info(
        "Supported surface format: {}, supported colorspace: {}.",
        Formats[surfFormat.format],
        ColorSpaces[surfFormat.colorSpace]);
  }

  MultiLogger::get()->info(
      "Swapchain composite alpha: {}",
      CompositeAlphaFlags[createInfo.compositeAlpha]);
  MultiLogger::get()->info(
      "Swapchain transform: {}", TransformFlags[createInfo.preTransform]);
  vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchainHandle);
  uint32_t swapImageCount{};
  vkGetSwapchainImagesKHR(device, swapchainHandle, &swapImageCount, nullptr);
  swapImages.resize(swapImageCount);
  vkGetSwapchainImagesKHR(
      device, swapchainHandle, &swapImageCount, swapImages.data());
}

Swapchain& Swapchain::operator=(Swapchain&& other) {
  if (this != &other) {
    device = other.device;
    swapchainHandle = other.swapchainHandle;
    swapImages = std::move(other.swapImages);
    swapExtent = std::move(other.swapExtent);
    other.device = {};
    other.swapchainHandle = {};
    other.swapExtent = {};
  }
  return *this;
}

Swapchain::Swapchain(Swapchain&& other) { *this = std::move(other); }

Swapchain::operator VkSwapchainKHR() { return swapchainHandle; }
Swapchain::operator VkSwapchainKHR*() { return &swapchainHandle; }

std::vector<VkImage> Swapchain::getSwapImages() const noexcept {
  return swapImages;
}

outcome::result<uint32_t, VkResult> Swapchain::acquireImage(VkFence fence) {
  uint32_t imageIndex{};
  auto result = vkAcquireNextImageKHR(
      device, swapchainHandle, UINT64_MAX, 0, fence, &imageIndex);
  if (result != VK_SUCCESS) {
    return outcome::failure(result);
  }
  return outcome::success(imageIndex);
}

VkExtent2D Swapchain::getSwapExtent() const noexcept { return swapExtent; }

Swapchain::~Swapchain() {
  if (device != VK_NULL_HANDLE && swapchainHandle != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, swapchainHandle, nullptr);
  }
}
}  // namespace vka