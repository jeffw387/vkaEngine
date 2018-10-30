#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace vka {
class Buffer {
public:
  Buffer(
      VmaAllocator allocator,
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VmaMemoryUsage memoryUsage,
      std::vector<uint32_t> queueIndices);
  ~Buffer();
  operator VkBuffer() const noexcept;
  operator VmaAllocation() const noexcept;
  uint32_t memType();
  VkDeviceMemory deviceMemory();
  VkDeviceSize offset();
  VkDeviceSize size();
  void* map();
  void unmap();
  void flush();
  void invalidate();

private:
  VmaAllocator allocator;
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo getAllocationInfo();
};
}  // namespace vka