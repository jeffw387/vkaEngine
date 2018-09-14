#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "VulkanFunctionLoader.hpp"
#include "vk_mem_alloc.h"

namespace vka {
class BufferDependent {
public:
  virtual void notify() = 0;
};

class Device;

class DeviceBuffer {
public:
  DeviceBuffer() = default;
  DeviceBuffer(
      VkDeviceSize size,
      VmaAllocator allocator,
      VmaMemoryUsage usage,
      VkBufferUsageFlags bufferUsage,
      VmaPool pool = VK_NULL_HANDLE);

  ~DeviceBuffer();

private:
  VmaAllocator allocator;
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;
};

template <typename T, size_t N>
class Buffer {
public:
  constexpr size_t bufferSize() { return sizeof(T) * N; };
  void setData(uint32_t frameIndex, std::array<T, N> data) {
    storage[frameIndex] = data;
    notifyDependents();
  }
  void setData(uint32_t frameIndex, size_t dataIndex, T data) {
    storage[frameIndex][dataIndex] = data;
    notifyDependents();
  }

  const std::array<T, N>& getData(uint32_t frameIndex);
  const T& getData(uint32_t frameIndex, size_t dataIndex);
  void addDependent(std::shared_ptr<BufferDependent> dependent) {
    dependents.push_back(dependent);
  }

private:
  void notifyDependents() {
    for (auto& dependent : dependents) {
      auto sharedDep = dependent.lock();
      sharedDep->notify();
    }
  }
  std::array<std::array<T, N>, 3> storage;
  std::vector<std::weak_ptr<BufferDependent>> dependents;
};
}  // namespace vka