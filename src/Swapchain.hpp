#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "outcome.hpp"
#include "VkEnumStrings.hpp"
#include "VkResult.hpp"

namespace vka {
namespace outcome = OUTCOME_V2_NAMESPACE;

class SwapchainCreateInfo {
public:
  SwapchainCreateInfo();
  operator const VkSwapchainCreateInfoKHR&();
  void setSurface(VkSurfaceKHR surface);
  void setMinImageCount(uint32_t minImageCount);
  void setImageFormat(VkFormat imageFormat);
  void setImageColorSpace(VkColorSpaceKHR imageColorSpace);
  void setImageExtent(VkExtent2D imageExtent);
  void addQueueFamilyIndex(uint32_t queueFamilyIndex);
  void setSurfacePreTransform(VkSurfaceTransformFlagBitsKHR preTransform);
  void setPresentMode(VkPresentModeKHR presentMode);
  void setOldSwapchain(VkSwapchainKHR oldSwapchain);

private:
  std::vector<uint32_t> queueFamilyIndices;
  VkSwapchainCreateInfoKHR createInfo;
};

class Swapchain {
public:
  Swapchain(
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      uint32_t graphicsQueueIndex,
      const VkSwapchainCreateInfoKHR& createInfo);
  ~Swapchain();
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(Swapchain&&);
  Swapchain(Swapchain&&);
  operator VkSwapchainKHR();
  operator VkSwapchainKHR*();

  std::vector<VkImage> getSwapImages() const noexcept;
  outcome::result<uint32_t, VkResult> acquireImage(VkFence fence);
  VkExtent2D getSwapExtent() const noexcept;

private:
  VkDevice device = VK_NULL_HANDLE;
  VkSwapchainKHR swapchainHandle = VK_NULL_HANDLE;
  std::vector<VkImage> swapImages;
  VkExtent2D swapExtent = {};
};
}  // namespace vka