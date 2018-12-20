#include "States.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <future>
#include <thread>

TEST_CASE("Add state to history") {
  States<int, 3> states;
  states.add(0, {});

  auto state = states.latest();

  REQUIRE(state);
  REQUIRE(**state == 0);
}

TEST_CASE("State points to data, future indicates when data is available") {
  
}