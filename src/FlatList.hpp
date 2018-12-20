#pragma once
#include <array>
#include <mutex>
#include <functional>
#include <optional>
#include <iterator>

template <typename T, size_t S>
class FlatList
{
public:
  struct iterator {
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using value_type = T;
    using reference = T&;
    
    pointer data = {};
    size_t index = {};

    bool operator !=(const iterator& other) const {
      return index != other.index;
    }

    bool operator ==(const iterator& other) const {
      return !(*this != other);
    }

    bool operator <(const iterator& other) const {
      return index < other.index;
    }

    bool operator >(const iterator& other) const {
      return index > other.index;
    }

    bool operator>=(const iterator& other) const { return !(*this) < other; }

    bool operator<=(const iterator& other) const { return !(*this) > other; }

    reference operator*() const { return data[index]; }
    pointer operator->() { return &(data[index]); }

    iterator& operator++() {
      ++index;
      if (index == S) {
        index -= S;
      }
      return *this;
    }

    iterator& operator--() {
      if (index == 0) {
        index += S;
      }
      else {
        --index;
      }
      return *this;
    }
  };

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
        if (auto first = first()) {
          if (p(first.value())) {
              return first;
          }
        }
        return {};
    }

    // pushes the given T to the end of the queue if space is available, returns false otherwise
    bool push_last(T newValue)
    {
        if (!(size() < S))
            return false;
        *m_end = std::move(newValue);
        ++m_end;
        ++m_size;
        return true;
    }

    size_t size()
    {
        return m_size;
    }

    size_t capacity()
    {
        return S;
    }

  private:

    std::array<T, S> storage;
    iterator m_begin = {storage.data(), 0};
    iterator m_end = {storage.data(), 0};
    size_t m_size = 0;
};