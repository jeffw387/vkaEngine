#pragma once
#include <memory>
#include <iterator>
#include <cstring>
#include <utility>
#include "Device.hpp"
#include "vk_mem_alloc.h"

namespace vka {
template <typename T, size_t N = 4U>
class vulkan_vector {
public:
  using iterator = T*;
  using const_iterator = const T*;
  vulkan_vector() = default;
  vulkan_vector(
      Device* device,
      VkBufferUsageFlags buffer_usage,
      VmaMemoryUsage memory_usage)
      : m_device(device),
        m_buffer_usage(buffer_usage),
        m_memory_usage(memory_usage) {}

  vulkan_vector<T>& operator=(const vulkan_vector<T>& other) {
    std::memcpy(m_storage, other.m_storage, other.m_size * sizeof(T));
  }
  vulkan_vector<T>& operator=(vulkan_vector<T>&& other) {
    std::swap(m_storage, other.m_storage);
  }

  T& operator[](size_t pos) { return m_storage[pos]; }
  const T& operator[](size_t pos) const { return m_storage[pos]; }
  T& front() { return m_storage[0]; }
  const T& front() const { return m_storage[0]; }
  T& back() { return m_storage[m_size - 1]; }
  const T& back() const { return m_storage[m_size - 1]; }
  T* data() noexcept { return m_storage; }
  const T* data() const noexcept { return m_storage; }
  iterator begin() noexcept { return m_storage; }
  const_iterator begin() const noexcept { return m_storage; }
  std::reverse_iterator<iterator> rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }
  std::reverse_iterator<const_iterator> rbegin() const noexcept {
    return std::make_reverse_iterator(end());
  }
  iterator end() noexcept { return m_storage + m_size; }
  const_iterator end() const noexcept { return m_storage + m_size; }
  std::reverse_iterator<iterator> rend() noexcept {
    return std::make_reverse_iterator(begin());
  }
  std::reverse_iterator<const_iterator> rend() const noexcept {
    return std::make_reverse_iterator(begin());
  }
  bool empty() const noexcept { return m_size == 0; }
  size_t size() const noexcept { return m_size; }
  void reserve(size_t new_cap) {
    if (new_cap > m_capacity) {
      auto actualNewCap = new_cap < N ? N : new_cap;
      auto newBuffer = m_device->createAllocatedBuffer(
          actualNewCap * sizeof(T), m_buffer_usage, m_memory_usage);
      void* newStoragePtr{};
      vmaMapMemory(
          m_device->getAllocator(), newBuffer.get().allocation, &newStoragePtr);
      std::memcpy(newStoragePtr, m_storage, m_size * sizeof(T));
      m_vulkan_buffer = std::move(newBuffer);
      m_storage = reinterpret_cast<T*>(newStoragePtr);
      m_capacity = actualNewCap;
    }
  }
  size_t capacity() const noexcept { return m_capacity; }
  void clear() noexcept {
    while (m_size > 0) {
      pop_back();
    }
  }
  void push_back(const T& value) {
    auto newPos = m_size;
    if (!(m_size < m_capacity)) {
      reserve(m_capacity * 2);
    }
    m_storage[newPos] = value;
    ++m_size;
  }
  void push_back(T&& value) {
    auto newPos = m_size;
    if (!(m_size < m_capacity)) {
      reserve(m_capacity * 2);
    }
    m_storage[newPos] = std::move(value);
    ++m_size;
  }
  void pop_back() { m_storage[--m_size].~T(); }
  void resize(size_t count) {
    reserve(count);
    if (count > m_size) {
      do {
        push_back(T());
      } while (count > m_size);
    } else if (count < m_size) {
      do {
        pop_back();
      } while (count < m_size);
    }
  }
  void resize(size_t count, const T& value) {
    reserve(count);
    if (count > m_size) {
      do {
        push_back(value);
      } while (count > m_size);
    } else if (count < m_size) {
      do {
        pop_back();
      } while (count < m_size);
    }
  }

private:
  T* m_storage = nullptr;
  size_t m_size = 0U;
  size_t m_capacity = 0U;
  Device* m_device;
  UniqueAllocatedBuffer m_vulkan_buffer;
  VkBufferUsageFlags m_buffer_usage = 0;
  VmaMemoryUsage m_memory_usage = VmaMemoryUsage(0);
};
}  // namespace vka