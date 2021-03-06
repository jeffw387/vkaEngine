#include <vulkan/vulkan.h>
#include <memory>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>
#include "physical_device.hpp"
#include "queue_family.hpp"

namespace vka {
static const char* swapchain_extension = "VK_KHR_swapchain";

struct device {
  explicit device(VkDevice device) : m_device(device) {}
  ~device() { vkDestroyDevice(m_device, nullptr); }
  device(const device&) = delete;
  device(device&&) = default;
  device& operator=(const device&) = delete;
  device& operator=(device&&) = default;
  operator VkDevice() const noexcept { return m_device; }

private:
  VkDevice m_device = {};
};

struct device_builder {
  tl::expected<std::unique_ptr<device>, VkResult> build(
      VkInstance instance) {
    VkDevice device = {};
    m_createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueInfos.size());
    m_createInfo.pQueueCreateInfos = queueInfos.data();
    m_createInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    m_createInfo.ppEnabledExtensionNames =
        extensions.data();
    m_createInfo.pEnabledFeatures = &features;

    auto result = vkCreateDevice(
        m_physicalDevice, &m_createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<vka::device>(device);
  }

  device_builder& add_queue_family(
      const queue_family& queueFamily) {
    VkDeviceQueueCreateInfo info{
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    info.queueCount = static_cast<uint32_t>(
        queueFamily.queuePriorities.size());
    info.pQueuePriorities =
        queueFamily.queuePriorities.data();
    info.queueFamilyIndex = queueFamily.familyIndex;
    queueInfos.push_back(std::move(info));
    return *this;
  }

  device_builder& physical_device(
      VkPhysicalDevice physicalDevice) {
    m_physicalDevice = physicalDevice;
    return *this;
  }

  device_builder& feature(device_features feature) {
    to_vulkan_feature(features, feature);
    return *this;
  }

  device_builder& extension(std::string_view name) {
    extensions.push_back(name.data());
    return *this;
  }

private:
  VkPhysicalDevice m_physicalDevice = {};
  std::vector<VkDeviceQueueCreateInfo> queueInfos = {};
  std::vector<const char*> extensions = {};
  VkPhysicalDeviceFeatures features = {};
  VkDeviceCreateInfo m_createInfo = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
};
}  // namespace vka