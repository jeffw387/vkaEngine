#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include <string>
#include <system_error>
#include "outcome.hpp"
#include "VkEnumStrings.hpp"

namespace std {
template <>
struct is_error_code_enum<VkResult> : std::true_type {};
}  // namespace std

namespace detail {
class VkResult_category : public std::error_category {
public:
  virtual const char* name() const noexcept override final {
    return "VkResult";
  }
  virtual std::string message(int result) const override final {
    return "unknown VkResult";
  }
};
}  // namespace detail

inline const detail::VkResult_category& VkResult_category() {
  static detail::VkResult_category c;
  return c;
}

inline std::error_code make_error_code(VkResult r) {
  return {static_cast<int>(r), VkResult_category()};
}

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
  ~Swapchain();
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(Swapchain&&);
  Swapchain(Swapchain&&);
  operator VkSwapchainKHR();
  operator VkSwapchainKHR*();

  const std::vector<VkImage>& getSwapImages();
  outcome::result<uint32_t, VkResult> acquireImage(VkFence fence);

private:
  VkDevice device = VK_NULL_HANDLE;
  VkSwapchainKHR swapchainHandle = VK_NULL_HANDLE;
  std::vector<VkImage> swapImages;
};
}  // namespace vka