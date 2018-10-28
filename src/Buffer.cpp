#include "Buffer.hpp"

namespace vka {
Buffer::Buffer(VmaAllocator allocator,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    std::vector<uint32_t> queueIndices) : allocator(allocator) {
      VkBufferCreateInfo bufferCreateInfo{};
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.usage = usage;
      bufferCreateInfo.size = size;
      bufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueIndices.size());
      bufferCreateInfo.pQueueFamilyIndices = queueIndices.data();
      bufferCreateInfo.sharingMode = queueIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
      
      VmaAllocationCreateInfo allocationCreateInfo{};
      allocationCreateInfo.usage = memoryUsage;
      
      vmaCreateBuffer(allocator,
        &bufferCreateInfo,
        &allocationCreateInfo,
        &buffer,
        &allocation,
        nullptr);
    }

Buffer::~Buffer() {
  if (allocator != nullptr && buffer != VK_NULL_HANDLE && allocation != nullptr) {
    vmaDestroyBuffer(allocator, buffer, allocation);
  }
}

Buffer::operator VkBuffer() const noexcept { return buffer; }
Buffer::operator VmaAllocation() const noexcept { return allocation; }

VmaAllocationInfo Buffer::getAllocationInfo() {
  VmaAllocationInfo info{};
  vmaGetAllocationInfo(allocator, allocation, &info);
  return std::move(info);
}

uint32_t Buffer::memType() {
  getAllocationInfo().memoryType;
}

VkDeviceMemory Buffer::deviceMemory() {
  return getAllocationInfo().deviceMemory;
}

VkDeviceSize Buffer::offset() {
  return getAllocationInfo().offset;
}

VkDeviceSize Buffer::size
}