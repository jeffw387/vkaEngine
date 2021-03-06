#pragma once
#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <mutex>
#include <optional>

template <typename T, size_t S>
struct flatlist_iterator {
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using value_type = T;
  using reference = T&;

  pointer data = {};
  size_t index = {};

  bool operator!=(const flatlist_iterator& other) const {
    return index != other.index;
  }

  bool operator==(const flatlist_iterator& other) const {
    return !(*this != other);
  }

  bool operator<(const flatlist_iterator& other) const {
    return index < other.index;
  }

  bool operator>(const flatlist_iterator& other) const {
    return index > other.index;
  }

  bool operator>=(const flatlist_iterator& other) const {
    return !(*this) < other;
  }

  bool operator<=(const flatlist_iterator& other) const {
    return !(*this) > other;
  }

  operator pointer() { return &data[index]; }
  reference operator*() const { return data[index]; }
  pointer operator->() { return &(data[index]); }

  flatlist_iterator& operator++() {
    ++index;
    if (index == S) {
      index -= S;
    }
    return *this;
  }

  flatlist_iterator& operator--() {
    if (index == 0) {
      index += S;
    }
    --index;
    return *this;
  }
};

template <typename T, size_t S>
class FlatList {
public:
  using iterator = flatlist_iterator<T, S>;

  iterator begin() { return m_begin; }
  iterator end() { return m_end; };

  // returns the first T if it exists
  std::optional<T> first() const {
    if (m_size == 0) {
      return {};
    }
    return {*m_begin};
  }

  std::optional<T> last() const {
    if (m_size > 0) {
      return *last_iterator();
    }
    return {};
  }

  iterator last_iterator() const {
    auto lastIt = m_end;
    --lastIt;
    return lastIt;
  }

  void pop_first() {
    if (m_size > 0) {
      m_begin->~T();
      ++m_begin;
      --m_size;
    }
  }

  void pop_last() {
    if (m_size > 0) {
      auto lastIt = last_iterator();
      lastIt->~T();
      --m_end;
      --m_size;
    }
  }

  template <typename PredicateT>
  std::optional<T> first_if(PredicateT p) {
    if (auto firstOptional = first()) {
      if (p(firstOptional.value())) {
        return firstOptional;
      }
    }
    return {};
  }

  // pushes the given T to the end of the queue if space is
  // available, returns false otherwise
  bool push_last(T newValue) {
    if (!(m_size < S)) return false;
    new (m_end) T(std::move(newValue));
    ++m_end;
    ++m_size;
    return true;
  }

  size_t size() const noexcept { return m_size; }

  constexpr size_t capacity() const noexcept { return S; }

private:
  std::array<std::byte, sizeof(T) * S> storage;
  iterator m_begin = {reinterpret_cast<T*>(storage.data()),
                      0};
  iterator m_end = {reinterpret_cast<T*>(storage.data()),
                    0};
  size_t m_size = 0;
};