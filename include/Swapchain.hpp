#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
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
  Swapchain() = default;
  Swapchain(
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      uint32_t graphicsQueueIndex,
      const VkSwapchainCreateInfoKHR& createInfo);
  const std::vector<VkImage>& getSwapImages();
  ~Swapchain();
  operator VkSwapchainKHR();
  operator VkSwapchainKHR*();

  outcome::result<uint32_t, VkResult> acquireImage(VkSemaphore semaphore);

private:
  VkDevice device = VK_NULL_HANDLE;
  VkSwapchainKHR swapchainHandle = VK_NULL_HANDLE;
  std::vector<VkImage> swapImages;
};
}  // namespace vka