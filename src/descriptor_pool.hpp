#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <expected.hpp>
#include <tuple>
// #include "VkResult.hpp"

namespace vka {
class descriptor_pool {
public:
  descriptor_pool(VkDevice device, VkDescriptorPool pool, bool individualResetAllowed)
      : m_device(device), m_pool(pool), m_individualResetAllowed(individualResetAllowed) {}
  operator VkDescriptorPool() { return m_pool; }
  ~descriptor_pool() { vkDestroyDescriptorPool(m_device, m_pool, nullptr); }
  bool individual_reset_allowed() const noexcept { return m_individualResetAllowed; }

private:
  VkDevice m_device = {};
  VkDescriptorPool m_pool = {};
  bool m_individualResetAllowed = {};
};

class descriptor_pool_builder {
public:
   tl::expected<std::unique_ptr<descriptor_pool>, VkResult> build(VkDevice device) {
    VkDescriptorPool pool{};
    m_createInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
    m_createInfo.pPoolSizes = m_poolSizes.data();
    m_createInfo.flags = m_individualResetAllowed ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
    auto result = vkCreateDescriptorPool(device, &m_createInfo, nullptr, &pool);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<descriptor_pool>(device, pool, m_individualResetAllowed);
  }

  descriptor_pool_builder& max_sets(uint32_t count) {
    m_createInfo.maxSets = count;
    return *this;
  }

  descriptor_pool_builder& add_type(VkDescriptorPoolSize pool_size) {
    m_poolSizes.push_back(std::move(pool_size));
    return *this;
  }

  descriptor_pool_builder& allow_individual_reset(bool is_allowed) {
    m_individualResetAllowed = is_allowed;
    return *this;
  }

private:
  std::vector<VkDescriptorPoolSize> m_poolSizes = {};
  bool m_individualResetAllowed = {};
  VkDescriptorPoolCreateInfo m_createInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
};
}  // namespace vka