#pragma once
#include <array>
#include <optional>

template <typename T>
struct Pooled {
  T* object = {};
  size_t index = {};

  operator T*() { return object; }
  operator bool() const { return object != nullptr; }
  T* get() { return object; }
  T value() const { return *object; }
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
        return Pooled<T>{&storage[resultIndex], resultIndex};
      }
    }
    return {};
  }

  void free(Pooled<T>& pooled) noexcept {
    if (pooled) {
      allocated[pooled.index] = false;
      pooled = {};
    }
  }
};