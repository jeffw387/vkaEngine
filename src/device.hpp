#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string_view>
#include <expected.hpp>
#include "physical_device.hpp"

namespace vka {



struct device {

struct device_builder {
  tl::expected<std::unique_ptr<device>, VkResult> build(VkInstance instance) {
    VkDevice device = {};
    m_createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    m_createInfo.pQueueCreateInfos = queueInfos.data();
    m_createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    m_createInfo.ppEnabledExtensionNames = extensions.data();
    m_createInfo.pEnabledFeatures = &features;
    
    auto result = vkCreateDevice(m_physicalDevice, &m_createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
  }

  void add_queue_family() {
    VkDeviceQueueCreateInfo info {};
    // info.
  }

  void physical_device(VkPhysicalDevice physicalDevice) {
    m_physicalDevice = physicalDevice;
  }

  void feature(device_features feature) {
    to_vulkan_feature(features, feature);
  }

  void extension(std::string_view name) {
    extensions.push_back(name.data());
  }
  
private:
  VkPhysicalDevice m_physicalDevice = {};
  std::vector<VkDeviceQueueCreateInfo> queueInfos = {};
  std::vector<const char*> extensions = {};
  VkPhysicalDeviceFeatures features = {};
  VkDeviceCreateInfo m_createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
};
}