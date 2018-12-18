#pragma once
#include <array>
#include <optional>

template <typename T>
class Pooled {
  friend class Pool;
  T* object = {};
  size_t index = {};

public:
  operator T*() { return object; }
  operator bool() { return object != nullptr; }
  T* get() { return object; }
};

template <typename T, size_t N>
class Pool {
  std::array<T, N> storage;
  std::array<bool, N> allocated;

public:
  [[nodiscard]] Pooled<T> allocate() noexcept {
    for (size_t resultIndex{}; resultIndex < N; ++resultIndex) {
      if (!allocated[resultIndex]) {
        allocated[resultIndex] = true;
        return Pooled{&storage[resultIndex], resultIndex});
      }
    }
    return {};
  }

  void free(Pooled<T> pooled) noexcept {
    if (pooled.get() != nullptr) {
      allocated[index] = false;
    }
  }
};