#include "States.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <future>
#include <thread>

TEST_CASE("Add state to history") {
  States<int, 3> states;
  states.add({});

  auto state = states.latest();

  REQUIRE(state);
}

TEST_CASE("Sync data") {
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
  int result = state->data;
  REQUIRE(result == 3);
}