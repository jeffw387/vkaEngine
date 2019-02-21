#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>

namespace vka {
struct allocator {
  explicit allocator(VmaAllocator allocator)
      : m_allocator(allocator) {}

  allocator(const allocator&) = delete;
  allocator(allocator&&) = default;
  allocator& operator=(const allocator&) = delete;
  allocator& operator=(allocator&&) = default;

  ~allocator() noexcept {
    vmaDestroyAllocator(m_allocator);
  }

  operator VmaAllocator() const noexcept {
    return m_allocator;
  }

private:
  VmaAllocator m_allocator = {};
};

struct allocator_builder {
  tl::expected<std::unique_ptr<allocator>, VkResult>
  build() {
    VmaAllocatorCreateInfo createInfo = {};
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.preferredLargeHeapBlockSize =
        m_preferredBlockSize;

    VmaAllocator allocatorHandle = {};
    auto result =
        vmaCreateAllocator(&createInfo, &allocatorHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<allocator>(allocatorHandle);
  }

  allocator_builder& physical_device(
      VkPhysicalDevice physicalDevice) {
    m_physicalDevice = physicalDevice;
    return *this;
  }

  allocator_builder& device(VkDevice deviceHandle) {
    m_device = deviceHandle;
    return *this;
  }

  allocator_builder& preferred_block_size(
      VkDeviceSize size) {
    m_preferredBlockSize = size;
    return *this;
  }

private:
  VkPhysicalDevice m_physicalDevice = {};
  VkDevice m_device = {};
  VkDeviceSize m_preferredBlockSize = {};
};
}  // namespace vka