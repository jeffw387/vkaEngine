#pragma once

#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <vk_mem_alloc.h>
#include <memory>

namespace vka {
struct buffer {
  explicit buffer(
      VmaAllocator allocator,
      VmaAllocation allocation,
      VkBuffer bufferHandle)
      : m_allocator(allocator),
        m_allocation(allocation),
        m_buffer(bufferHandle) {}

  buffer(const buffer&) = delete;
  buffer(buffer&&) = default;
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&&) = default;

  ~buffer() noexcept { vmaDestroyBuffer(m_allocator, m_buffer, m_allocation); }

  operator VkBuffer() { return m_buffer; }
  operator VmaAllocation() { return m_allocation; }

  tl::expected<void*, VkResult> map() noexcept {
    if (!m_mapped) {
      auto result = vmaMapMemory(m_allocator, m_allocation, &m_mapPtr);
      if (result != VK_SUCCESS) {
        return tl::make_unexpected(result);
      }
      m_mapped = true;
    }
    return m_mapPtr;
  }

  void unmap() noexcept {
    if (m_mapped) {
      vmaUnmapMemory(m_allocator, m_allocation);
      m_mapped = false;
    }
  }

private:
  VmaAllocator m_allocator = {};
  VmaAllocation m_allocation = {};
  VkBuffer m_buffer = {};
  bool m_mapped = {};
  void* m_mapPtr = {};
};

struct buffer_builder {
  tl::expected<std::unique_ptr<buffer>, VkResult> build(
      VmaAllocator allocator) {
    VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.usage = m_bufferUsage;
    bufferCreateInfo.size = m_bufferSize;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 1;
    bufferCreateInfo.pQueueFamilyIndices = &m_queueFamilyIndex;

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = m_allocationFlags;
    allocationCreateInfo.usage = m_memoryUsage;
    allocationCreateInfo.pool = m_memoryPool;

    VmaAllocation allocation = {};
    VkBuffer bufferHandle = {};
    auto result = vmaCreateBuffer(
        allocator,
        &bufferCreateInfo,
        &allocationCreateInfo,
        &bufferHandle,
        &allocation,
        nullptr);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<buffer>(allocator, allocation, bufferHandle);
  }

  buffer_builder& dedicated() {
    m_allocationFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    return *this;
  }

  buffer_builder& memory_pool(VmaPool memoryPool) {
    m_memoryPool = memoryPool;
    return *this;
  }

  buffer_builder& transfer_source() {
    m_bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    return *this;
  }

  buffer_builder& transfer_destination() {
    m_bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return *this;
  }

  buffer_builder& uniform_buffer() {
    m_bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    return *this;
  }

  buffer_builder& storage_buffer() {
    m_bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    return *this;
  }

  buffer_builder& index_buffer() {
    m_bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    return *this;
  }

  buffer_builder& vertex_buffer() {
    m_bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    return *this;
  }

  buffer_builder& indirect_buffer() {
    m_bufferUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    return *this;
  }

  buffer_builder& size(VkDeviceSize bufferSize) {
    m_bufferSize = bufferSize;
    return *this;
  }

  buffer_builder& cpu_only() {
    m_memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    return *this;
  }

  buffer_builder& gpu_only() {
    m_memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    return *this;
  }

  buffer_builder& cpu_to_gpu() {
    m_memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    return *this;
  }

  buffer_builder& gpu_to_cpu() {
    m_memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    return *this;
  }

  buffer_builder& queue_family_index(uint32_t index) {
    m_queueFamilyIndex = index;
    return *this;
  }

private:
  VkDeviceSize m_bufferSize = {};
  VkBufferUsageFlags m_bufferUsage = {};
  VmaAllocationCreateFlags m_allocationFlags = {};
  VmaMemoryUsage m_memoryUsage = {};
  VmaPool m_memoryPool = {};
  uint32_t m_queueFamilyIndex = {};
};
}  // namespace vka