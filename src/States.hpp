#include <future>
#include "ObjectPool.hpp"
#include "FlatList.hpp"

template <typename T>
struct State {
  std::shared_future<T> future;
  Pooled<T> data;

  T value() const { return data.value(); }
  T& value() { return data.value(); }
  void sync() {
    data.value() = future.get();
  }
};

template <typename T, size_t N>
class States {
  Pool<T, N> pool;
  FlatList<State<T>, N> history;

  void expire_oldest() {
    auto first = history.first();
    pool.free(first->data);
    history.pop_first();
  }

public:
  auto latest() {
    return history.last();
  }

  void add(std::shared_future<T> future) {
    if (history.size() == N) {
      expire_oldest();
    }
    auto new_pooled = pool.allocate();
    history.push_last(State<T>{future, new_pooled});
  }
};