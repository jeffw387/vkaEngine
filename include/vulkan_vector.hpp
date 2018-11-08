#pragma once
#include <memory>
#include <iterator>
#include <cstring>
#include <utility>
#include <vulkan/vulkan.h>
#include <functional>
#include "Device.hpp"
#include "vk_mem_alloc.h"
#include "DescriptorPool.hpp"

namespace vka {

enum class VectorType {
  Uniform,
  DynamicUniform,
  Storage,
  // DynamicStorage
};
template <typename T, typename subscriberT = BufferDescriptor, size_t N = 4U>
class vulkan_vector {
public:
  using value_type = T;
  using subscriber_type = subscriberT*;
  struct iterator {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using value_type = T;
    using reference = T&;
    char* elementPtr;
    size_t alignment = 0U;

    reference operator*() { return *reinterpret_cast<pointer>(elementPtr); }

    bool operator!=(const iterator& other) {
      return (elementPtr != other.elementPtr || alignment != other.alignment);
    }

    bool operator==(const iterator& other) { return !(*this != other); }

    bool operator<(const iterator& other) {
      return elementPtr < other.elementPtr;
    }

    bool operator>(const iterator& other) {
      return elementPtr > other.elementPtr;
    }

    bool operator>=(const iterator& other) { return !(*this) < other; }

    bool operator<=(const iterator& other) { return !(*this) > other; }

    pointer operator->() { return reinterpret_cast<pointer>(elementPtr); }

    reference operator[](difference_type difference) {
      return *((*this) += difference);
    }

    iterator& operator++() {
      elementPtr += alignment;
      return *this;
    }

    iterator& operator++(int) {
      auto result = *this;
      ++(*this);
      return result;
    }

    iterator& operator--() {
      elementPtr -= alignment;
      return *this;
    }

    iterator& operator--(int) {
      auto result = *this;
      --(*this);
      return result;
    }

    iterator& operator+=(difference_type difference) {
      elementPtr += (alignment * difference);
      return *this;
    }

    iterator& operator-=(difference_type difference) {
      elementPtr -= (alignment * difference);
    }

    iterator operator+(difference_type difference) {
      auto result = (*this);
      return result += difference;
    }

    iterator operator-(difference_type difference) {
      auto result = (*this);
      return result -= difference;
    }

    difference_type operator-(const iterator& other) {
      return (elementPtr - other.elementPtr) / alignment;
    }
  };

  struct const_iterator {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    char* elementPtr;
    size_t alignment = 0U;

    reference operator*() { return *reinterpret_cast<pointer>(elementPtr); }

    bool operator!=(const const_iterator& other) {
      return (elementPtr != other.elementPtr || alignment != other.alignment);
    }

    bool operator==(const const_iterator& other) { return !(*this != other); }

    bool operator<(const const_iterator& other) {
      return elementPtr < other.elementPtr;
    }

    bool operator>(const const_iterator& other) {
      return elementPtr > other.elementPtr;
    }

    bool operator>=(const const_iterator& other) { return !(*this) < other; }

    bool operator<=(const const_iterator& other) { return !(*this) > other; }

    pointer operator->() { return reinterpret_cast<pointer>(elementPtr); }

    reference operator[](difference_type difference) {
      return *((*this) += difference);
    }

    const_iterator& operator++() {
      elementPtr += alignment;
      return *this;
    }

    const_iterator& operator++(int) {
      auto result = *this;
      ++(*this);
      return result;
    }

    const_iterator& operator--() {
      elementPtr -= alignment;
      return *this;
    }

    const_iterator& operator--(int) {
      auto result = *this;
      --(*this);
      return result;
    }

    const_iterator& operator+=(difference_type difference) {
      elementPtr += (alignment * difference);
      return *this;
    }

    const_iterator& operator-=(difference_type difference) {
      elementPtr -= (alignment * difference);
    }

    const_iterator operator+(difference_type difference) {
      auto result = (*this);
      return result += difference;
    }

    const_iterator operator-(difference_type difference) {
      auto result = (*this);
      return result -= difference;
    }

    difference_type operator-(const const_iterator& other) {
      return (elementPtr - other.elementPtr) / alignment;
    }
  };
  vulkan_vector() = default;
  vulkan_vector(
      Device* device,
      VkBufferUsageFlags buffer_usage,
      VmaMemoryUsage memory_usage,
      VectorType vector_type = VectorType::Uniform)
      : m_device(device),
        m_buffer_usage(buffer_usage),
        m_memory_usage(memory_usage),
        vector_type(vector_type) {
    if (vector_type == VectorType::DynamicUniform) {
      auto minDynamicUboAlignment =
          device->getDeviceProperties().limits.minUniformBufferOffsetAlignment;
      if (sizeof(T) < minDynamicUboAlignment) {
        m_alignment = minDynamicUboAlignment;
      } else {
        m_alignment = static_cast<VkDeviceSize>(
            std::ceil(double(sizeof(T)) / double(minDynamicUboAlignment)));
      }
    } else {
      m_alignment = sizeof(T);
    }
  }

  void subscribe(subscriber_type subscriber) {
    m_subscribers.push_back(subscriber);
    notify();
  }

  template <typename PtrT>
  iterator make_iterator(PtrT ptr) {
    return iterator{reinterpret_cast<char*>(ptr), m_alignment};
  }

  template <typename PtrT>
  const_iterator make_const_iterator(PtrT ptr) {
    return const_iterator{reinterpret_cast<char*>(ptr), m_alignment};
  }

  vulkan_vector(const vulkan_vector<T>&) = delete;
  vulkan_vector<T>& operator=(const vulkan_vector<T>&) = delete;
  vulkan_vector(vulkan_vector<T>&& other) { *this = std::move(other); }
  vulkan_vector<T>& operator=(vulkan_vector<T>&& other) {
    if (this != &other) {
      m_storage = other.m_storage;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      m_device = other.m_device;
      m_alignment = other.m_alignment;
      m_vulkan_buffer = std::move(other.m_vulkan_buffer);
      m_buffer_usage = other.m_buffer_usage;
      m_memory_usage = other.m_memory_usage;
      m_subscribers = std::move(other.m_subscribers);
    }
    return *this;
  }

  void flushMemory(Device* device) {
    vmaFlushAllocation(
        device->getAllocator(), *m_vulkan_buffer, 0, size() * m_alignment);
  }

  VkDeviceSize getDynamicOffset(size_t index) { return index * m_alignment; }
  T& operator[](size_t pos) { return begin()[pos]; }
  const T& operator[](size_t pos) const { return *(begin()[pos]); }
  T& front() { return (*this)[0]; }
  const T& front() const { return (*this)[0]; }
  T& back() { return (*this)[m_size - 1]; }
  const T& back() const { return (*this)[m_size - 1]; }
  T* data() noexcept { return (*this); }
  const T* data() const noexcept { return m_storage; }
  iterator begin() noexcept { return make_iterator(m_storage); }
  const_iterator begin() const noexcept {
    return make_const_iterator(m_storage);
  }
  std::reverse_iterator<iterator> rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }
  std::reverse_iterator<const_iterator> rbegin() const noexcept {
    return std::make_reverse_iterator(end());
  }
  iterator end() noexcept { return begin() + size(); }
  const_iterator end() const noexcept { return begin() + size(); }
  std::reverse_iterator<iterator> rend() noexcept {
    return std::make_reverse_iterator(begin());
  }
  std::reverse_iterator<const_iterator> rend() const noexcept {
    return std::make_reverse_iterator(begin());
  }
  bool empty() const noexcept { return size() == 0; }
  size_t size() const noexcept { return m_size; }

  void notify() {
    for (auto subscriber : m_subscribers) {
      (*subscriber)(*m_vulkan_buffer, size() * m_alignment);
    }
  }

  void reserve(size_t new_cap) {
    auto actualNewCap = new_cap < N ? N : new_cap;
    if (actualNewCap > m_capacity) {
      auto newBuffer = m_device->createBuffer(
          actualNewCap * m_alignment, m_buffer_usage, m_memory_usage);
      void* newStoragePtr = newBuffer->map();
      std::memcpy(newStoragePtr, m_storage, m_size * m_alignment);
      m_vulkan_buffer = std::move(newBuffer);
      m_storage = reinterpret_cast<T*>(newStoragePtr);
      m_capacity = actualNewCap;
      notify();
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
    (*this)[newPos] = value;
    ++m_size;
    if (vector_type != VectorType::Storage) {
      notify();
    }
  }
  void push_back(T&& value) {
    auto newPos = m_size;
    if (!(m_size < m_capacity)) {
      reserve(m_capacity * 2);
    }
    (*this)[newPos] = std::move(value);
    ++m_size;
    if (vector_type != VectorType::Storage) {
      notify();
    }
  }
  void pop_back() {
    (*this)[--m_size].~T();
    if (vector_type != VectorType::Storage) {
      notify();
    }
  }
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
  VkDeviceSize m_alignment;
  std::shared_ptr<Buffer> m_vulkan_buffer;
  VkBufferUsageFlags m_buffer_usage = 0;
  VmaMemoryUsage m_memory_usage = VmaMemoryUsage(0);
  std::vector<subscriber_type> m_subscribers;
  VectorType vector_type;
};
}  // namespace vka