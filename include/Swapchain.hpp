#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>
#include "outcome.hpp"

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

private:
  std::vector<uint32_t> queueFamilyIndices;
  VkSwapchainCreateInfoKHR createInfo;
};

class Swapchain {
public:
  Swapchain() = delete;
  Swapchain(VkDevice device, const VkSwapchainCreateInfoKHR& createInfo);
  const std::vector<VkImage>& getSwapImages();
  ~Swapchain();
  operator VkSwapchainKHR();
  operator VkSwapchainKHR*();

  outcome::result<uint32_t, VkResult> acquireImage(VkSemaphore semaphore);

private:
  VkDevice device;
  VkSwapchainKHR swapchainHandle;
  std::vector<VkImage> swapImages;
};
}  // namespace vka