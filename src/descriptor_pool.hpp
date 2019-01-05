#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <expected.hpp>
#include <tuple>
#include "VkResult.hpp"

namespace vka {
class descriptor_pool {
public:
  descriptor_pool(VkDevice device, VkDescriptorPool pool, bool individual_reset_allowed)
      : m_device(device), m_pool(pool), m_individual_reset_allowed(individual_reset_allowed) {}
  operator VkDescriptorPool() { return m_pool; }
  ~descriptor_pool() { vkDestroyDescriptorPool(m_device, m_pool, nullptr); }

private:
  VkDevice m_device = {};
  VkDescriptorPool m_pool = {};
  bool m_individual_reset_allowed = {};
};

class descriptor_pool_builder {
public:
   tl::expected<std::unique_ptr<descriptor_pool>, VkResult> build(VkDevice device) {
    VkDescriptorPool pool{};
    create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    create_info.pPoolSizes = pool_sizes.data();
    create_info.flags = individual_reset_allowed ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
    auto result = vkCreateDescriptorPool(device, &create_info, nullptr, &pool);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<descriptor_pool>(device, pool, individual_reset_allowed);
  }

  descriptor_pool_builder& max_sets(uint32_t count) {
    create_info.maxSets = count;
    return *this;
  }

  descriptor_pool_builder& add_type(VkDescriptorPoolSize pool_size) {
    pool_sizes.push_back(std::move(pool_size));
    return *this;
  }

  descriptor_pool_builder& allow_individual_reset(bool is_allowed) {
    individual_reset_allowed = is_allowed;
    return *this;
  }

private:
  std::vector<VkDescriptorPoolSize> pool_sizes = {};
  bool individual_reset_allowed = {};
  VkDescriptorPoolCreateInfo create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
};
}  // namespace vka