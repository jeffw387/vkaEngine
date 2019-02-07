#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <tl/expected.hpp>
#include <tuple>
#include "descriptor_set_layout.hpp"

namespace vka {
class descriptor_pool {
public:
  descriptor_pool(
      VkDevice device,
      VkDescriptorPool pool,
      bool individualResetAllowed)
      : m_device(device),
        m_pool(pool),
        m_individualResetAllowed(individualResetAllowed) {}
  operator VkDescriptorPool() { return m_pool; }
  ~descriptor_pool() { vkDestroyDescriptorPool(m_device, m_pool, nullptr); }
  bool individual_reset_allowed() const noexcept {
    return m_individualResetAllowed;
  }

private:
  VkDevice m_device = {};
  VkDescriptorPool m_pool = {};
  bool m_individualResetAllowed = {};
};


}  // namespace vka