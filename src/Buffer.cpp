#include "Buffer.hpp"

namespace vka {
DeviceBuffer::DeviceBuffer(
  VkDeviceSize size,
  VmaAllocator allocator,
  VmaMemoryUsage memUsage,
  VkBufferUsageFlags bufferUsage,
  VmaPool pool)
  : allocator(allocator) {
  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = bufferUsage;

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.pool = pool;
  allocCreateInfo.usage = memUsage;

  vmaCreateBuffer(
    allocator,
    &bufferCreateInfo,
    &allocCreateInfo,
    &buffer,
    &allocation,
    &allocationInfo);
}

DeviceBuffer::~DeviceBuffer() {
  vmaDestroyBuffer(allocator, buffer, allocation);
}
}  // namespace vka