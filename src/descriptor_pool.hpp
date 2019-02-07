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

inline auto make_pool(
    VkDevice device,
    std::vector<set_data> set_layouts,
    bool individualReset = false) {
  std::vector<VkDescriptorPoolSize> poolSizes;
  VkDescriptorPoolCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  for (const set_data& set : set_layouts) {
    const auto& [bindings, l, maxSets] = set;
    createInfo.maxSets += maxSets;
    for (const auto& binding : bindings) {
      const auto& [s, type, elements, i] = binding;
      poolSizes.push_back(
          {type, static_cast<uint32_t>(elements.size() * maxSets)});
    }
  }
  createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();
  VkDescriptorPool pool{};
  auto poolResult = vkCreateDescriptorPool(device, &createInfo, nullptr, &pool);
  if (poolResult != VK_SUCCESS) {
    exit(poolResult);
  }
  return std::make_unique<descriptor_pool>(device, pool, individualReset);
}
}  // namespace vka