#include <future>
#include "ObjectPool.hpp"
#include "CircularQueue.hpp"

template <typename T>
struct State {
  std::shared_future<T> future;
  Pooled<T> data;
};

template <typename T, size_t N>
struct States {
  Pool<T, N> pool;
  CircularQueue<State<T>, N> history;

  auto latest() {
    return history.readFirst();
  }

  void add(State<T> new_state) {
    if (history.size() == N) {
      history.popFirst()
    }
    history.pushLast(std::move(new_state));
  }
};