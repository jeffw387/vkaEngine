#pragma once
#include <array>
#include <mutex>
#include <functional>
#include <optional>

template <typename T, size_t S>
class CircularQueue
{
    std::array<T, S> storage;
    size_t firstID = 0;
    size_t endID = 0;
    size_t count = 0;
    mutable std::mutex storageMutex;

public:
    // returns the first T if it exists
    std::optional<T> readFirst() const
    {
        auto result = std::optional<T>();
        if (count == 0)
        {
            return result;
        }
        std::lock_guard<std::mutex> lock(storageMutex);
        result = storage[firstID];
        return result;
    }

    // removes and returns the first T if it exists
    std::optional<T> popFirst()
    {
        auto first = readFirst();
        std::optional<T> result;
        std::lock_guard<std::mutex> lock(storageMutex);
        if (!first.has_value())
            return result;
        result = first.value();
        auto newFirst = (++firstID) % S;
        count--;
        firstID = newFirst;
        return result;
    }
    
    template <typename PredicateType>
    std::optional<T> popFirstIf(PredicateType p) {
        std::optional<T> result;
        std::lock_guard<std::mutex> lock(storageMutex);
        if (auto first = readFirst()) {
          if (p(first.value())) {
              result = first.value();
              auto newFirst = (++firstID) % S;
              count--;
              firstID = newFirst;
          }
        }
        return result;
    }

    // pushes the given T to the end of the queue if space is available, returns false otherwise
    bool pushLast(T newValue)
    {
        std::lock_guard<std::mutex> lock(storageMutex);
        if (!(count < S))
            return false;
        storage[endID] = newValue;
        count++;
        auto newEnd = (++endID) % S;
        endID = newEnd;
        return true;
    }

    size_t size()
    {
        return count;
    }

    size_t capacity()
    {
        return S;
    }
};