#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include "move_into.hpp"

namespace vka {
struct queue_family {
  uint32_t familyIndex;
  std::vector<float> queuePriorities;
};

inline tl::optional<uint32_t> queue_flag_match(
    tl::optional<uint32_t> index,
    VkQueueFlags supported,
    VkQueueFlags required) {
  if ((supported & required) == required) {
    return index;
  }
  return {};
}

inline tl::optional<uint32_t> queue_present_match(
    tl::optional<uint32_t> index,
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

struct no_matching_queue_family {};

struct queue_family_builder {
  using result_type =
      tl::expected<queue_family, no_matching_queue_family>;
  result_type build(VkPhysicalDevice physicalDevice) {
    result_type result;
    uint32_t count = {};
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties = {};
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &count, properties.data());

    for (uint32_t i = {}; i < count; ++i) {
      const auto& prop = properties[i];
      queue_flag_match(i, prop.queueFlags, m_queueFlags)
          .and_then([&](auto value) {
            return queue_present_match(
                value,
                physicalDevice,
                m_surface,
                m_presentRequired);
          })
          .map(move_into{m_family.familyIndex})
          .or_else(move_value_into<result_type>{
              tl::make_unexpected(
                  no_matching_queue_family{}),
              result});
    }
    result = m_family;
    return result;
  }

  queue_family_builder& present_support(
      VkSurfaceKHR surface) {
    m_presentRequired = true;
    m_surface = surface;
    return *this;
  }

  queue_family_builder& graphics_support() {
    m_queueFlags |= VK_QUEUE_GRAPHICS_BIT;
    return *this;
  }

  queue_family_builder& compute_support() {
    m_queueFlags |= VK_QUEUE_COMPUTE_BIT;
    return *this;
  }

  queue_family_builder& transfer_support() {
    m_queueFlags |= VK_QUEUE_TRANSFER_BIT;
    return *this;
  }

  queue_family_builder& queue(float priority) {
    m_family.queuePriorities.push_back(priority);
    return *this;
  }

private:
  VkSurfaceKHR m_surface = {};
  queue_family m_family = {};
  VkQueueFlags m_queueFlags = {};
  bool m_presentRequired = {};
};
}  // namespace vka