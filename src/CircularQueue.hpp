#pragma once
#include <array>
#include <mutex>
#include <functional>
#include <optional>
#include <iterator>

template <typename T, size_t S>
class CircularQueue
{
public:
  struct iterator {
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using value_type = T;
    using reference = T&;
    
    T* data = {};
    size_t index = {};

    bool operator !=(const iterator& other) {
      return index != other.index;
    }

    bool operator ==(const iterator& other) {
      return !(*this != other);
    }

    bool operator <(const iterator& other) {
      return index < other.index;
    }

    bool operator >(const iterator& other) {
      return index > other.index;
    }

    bool operator>=(const iterator& other) { return !(*this) < other; }

    bool operator<=(const iterator& other) { return !(*this) > other; }

    reference operator*() { return data[index]; }
    pointer operator->() { return &(data[index]); }

    iterator& operator++() {
      ++index;
      if (index == S) {
        index -= S;
      }
      return *this;
    }

    iterator& operator++(int) {
      auto result = *this;
      ++(*this);
      return result;
    }

    iterator& operator--() {
      if (index == 0) {
        index += S;
      }
      return *this;
    }

    iterator& operator--(int) {
      auto result = *this;
      --(*this);
      return result;
    }
  };

    iterator begin() { return m_begin; }
    iterator end() { return m_end; };

    // returns the first T if it exists
    std::optional<T> first() const {
        if (m_size == 0) {
            return {};
        }
        return {*(begin());};
    }

    std::optional<T> last() const {
      if (m_size > 0) {
        return *(--end());
      }
    }

    void pop_first() {
      if (m_size > 0) {
        auto newFirstID = (++begin_index) % S;
        m_size--;
        begin_index = newFirstID;
      }
    }

    // removes and returns the first T if it exists
    std::optional<T> first() {
        return first;
        
    }
    
    template <typename PredicateType>
    std::optional<T> first_if(PredicateType p) {
        if (auto first = read_first()) {
          if (p(first.value())) {
              return first;

          }
        }
        return {};
    }

    // pushes the given T to the end of the queue if space is available, returns false otherwise
    bool push_last(T newValue)
    {
        if (!(m_size < S))
            return false;
        storage[end_index] = newValue;
        m_size++;
        auto newEnd = (++end_index) % S;
        end_index = newEnd;
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
    iterator m_begin;
    iterator m_end;
    size_t m_size = 0;
};