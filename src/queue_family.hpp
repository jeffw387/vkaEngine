#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace vka {
struct queue_family {
  uint32_t familyIndex;
  std::vector<float> queuePriorities;
};

inline std::optional<uint32_t> queue_flag_match(
    std::optional<uint32_t> index,
    VkQueueFlags supported,
    VkQueueFlags required) {
  if ((supported & required) == required) {
    return index;
  }
  return {};
}

inline std::optional<uint32_t> queue_present_match(
    std::optional<uint32_t> index,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkBool32 required) {
  if (!required) {
    return index;
  }
  if (!index) {
    return index;
  }
  VkBool32 presentSupported = {};
  auto result = vkGetPhysicalDeviceSurfaceSupportKHR(
      physicalDevice, *index, surface, &presentSupported);
  if (result != VK_SUCCESS) {
    exit(result);
  }
  if (presentSupported) {
    return index;
  }
  return {};
}

struct queue_family_builder {
  std::optional<queue_family> build(
      VkPhysicalDevice physicalDevice,
      VkSurfaceKHR surface) {
    uint32_t count = {};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties = {};
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &count, properties.data());

    for (uint32_t i = {}; i < count; ++i) {
      const auto& prop = properties[i];
      auto flags_optional = queue_flag_match(i, prop.queueFlags, m_queueFlags);
      auto present_optional = queue_present_match(
          flags_optional, physicalDevice, surface, m_presentRequired);
      if (present_optional) {
        m_family.familyIndex = *present_optional;
        return m_family;
      }
    }
    return {};
  }

  void present_support() { m_presentRequired = true; }

  void graphics_support() { m_queueFlags |= VK_QUEUE_GRAPHICS_BIT; }

  void compute_support() { m_queueFlags |= VK_QUEUE_COMPUTE_BIT; }

  void transfer_support() { m_queueFlags |= VK_QUEUE_TRANSFER_BIT; }

  void queue(float priority) { m_family.queuePriorities.push_back(priority); }

private:
  queue_family m_family = {};
  VkQueueFlags m_queueFlags = {};
  bool m_presentRequired = {};
};
}  // namespace vka