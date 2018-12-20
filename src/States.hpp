#include <future>
#include "ObjectPool.hpp"
#include "FlatList.hpp"

template <typename T>
struct State {
  std::shared_future<T> future;
  Pooled<T> data;

  T operator* () const { return data.value(); }
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

  void add(T new_state, std::shared_future<T> future) {
    if (history.size() == N) {
      expire_oldest();
    }
    auto new_pooled = pool.allocate();
    *new_pooled.get() = std::move(new_state);
    history.push_last(State<T>{std::move(future), std::move(new_pooled)});
  }
};