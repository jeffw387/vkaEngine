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

TEST_CASE("State points to data, future indicates when data is available") {
  States<int, 1> states;
  std::promise<int> int_promise;
  auto int_future = int_promise.get_future().share();
  states.add(int_future);
  auto worker_lambda = [](std::promise<int> work_promise) {
    work_promise.set_value(3);
  };
  std::thread worker{worker_lambda, std::move(int_promise)};

  auto state = states.latest();
  REQUIRE(state);
  state->sync();
  REQUIRE(states.latest()->data.value() == 3);
}