#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>
#include "descriptor_pool.hpp"

namespace vka {
struct descriptor_set {
  explicit descriptor_set(
    VkDevice device, 
    VkDescriptorPool pool, 
    VkDescriptorSet set, 
    bool allowIndividualReset) :
      m_device(device),
      m_pool(pool),
      m_set(set),
      m_allowIndividualReset(allowIndividualReset) {}
  descriptor_set(const descriptor_set&) = delete;
  descriptor_set(descriptor_set&&) = default;
  descriptor_set& operator=(const descriptor_set&) = delete;
  descriptor_set& operator=(descriptor_set&&) = default;
  ~descriptor_set() {
    vkFreeDescriptorSets(m_device, m_pool, 1, &m_set);
  }
  operator VkDescriptorSet() { return m_set; }
private:
  VkDevice m_device = {};
  VkDescriptorPool m_pool = {};
  VkDescriptorSet m_set = {};
  bool m_allowIndividualReset = {};
};

struct descriptor_set_allocator {
  tl::expected<std::unique_ptr<descriptor_set>, VkResult> allocate(VkDevice device, descriptor_pool& pool) {
    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_layout;
    
    VkDescriptorSet set = {};
    auto result = vkAllocateDescriptorSets(device, &allocateInfo, &set);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<descriptor_set>(
      device, 
      pool, 
      set, 
      pool.individual_reset_allowed());
  }

  descriptor_set_allocator& set_layout(VkDescriptorSetLayout layout) {
    m_layout = layout;
  }
private:
  VkDescriptorSetLayout m_layout;    
};
}