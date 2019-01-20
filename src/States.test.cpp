#include "States.hpp"

#include <catch2/catch.hpp>
#include <future>
#include <thread>

TEST_CASE("Add state to history") {
  States<int, 3> states;
  states.add({});

  auto state = states.latest();

  REQUIRE(state);
}

TEST_CASE("Sync data (single thread)") {
  States<int, 1> states;
  std::promise<int> int_promise;
  auto int_future = int_promise.get_future().share();
  states.add(int_future);
  auto int_future2 = int_future;

  REQUIRE_NOTHROW(int_promise.set_value(3));

  REQUIRE(int_future.valid());
  REQUIRE(int_future2.valid());
  REQUIRE_NOTHROW([&]() {
    auto result = int_future.get();
    REQUIRE(result == 3);
  }());

  REQUIRE_NOTHROW([&]() {
    auto result = int_future2.get();
    REQUIRE(result == 3);
  }());

  auto state = states.latest();
  REQUIRE(state);
  state->sync();
  int result = state->data.value();
  REQUIRE(result == 3);
}

TEST_CASE("Sync data (two threads)") {
  States<int, 1> states;
  std::promise<int> int_promise;
  auto int_future = int_promise.get_future().share();
  states.add(int_future);

  auto update_state = [](auto promise) { promise.set_value(3); };
  std::thread worker(update_state, std::move(int_promise));

  auto latest = states.latest();
  REQUIRE(latest);
  REQUIRE_NOTHROW(latest->sync());
  int result = latest->value();
  REQUIRE(result == 3);
  worker.join();
}

TEST_CASE("Roughly intended use case") {
  using Position = std::array<float, 2>;
  auto posUpdate = [](Position pos) {
    constexpr auto constantAccel = Position{0.f, 9.8f};
    pos[0] += constantAccel[0];
    pos[1] += constantAccel[1];
    return pos;
  };
  States<Position, 3> positionStates;
  auto initialPos =
      std::packaged_task<Position(Position)>([](auto pos) { return pos; });
  positionStates.add(initialPos.get_future());
  initialPos(Position{});

  auto updateLoop = [&]() {
    for (int updateCount{}; updateCount < 5; ++updateCount) {
      if (auto latest = positionStates.latest()) {
        auto update_task = std::packaged_task<Position(Position)>(posUpdate);
        latest->sync();
        auto positionFuture = update_task.get_future().share();
        positionStates.add(positionFuture);
        std::thread worker{std::move(update_task), latest->value()};
        worker.join();
      }
    }
  };
  REQUIRE_NOTHROW(updateLoop());
  positionStates.latest()->sync();
  REQUIRE(positionStates.latest()->value()[1] == 9.8f * 5.f);
}