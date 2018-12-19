#include "CircularQueue.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Adding item to queue") {
  CircularQueue<int, 3> queue;
  REQUIRE(queue.size() == 0);
  REQUIRE(queue.capacity() == 3);

  auto push_result = queue.push_last(1);
  REQUIRE(queue.size() == 1);
  REQUIRE(push_result == true);
}

TEST_CASE("Adding an item when at capacity should fail") {
  CircularQueue<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(1);
  push_result = queue.push_last(1);

  REQUIRE(push_result == true);

  push_result = queue.push_last(1);
  REQUIRE(push_result == false);
}

TEST_CASE("Read first item in queue with size 1") {
  CircularQueue<int, 3> queue;
  auto push_result = queue.push_last(1);
  auto first = queue.first();

  REQUIRE(first);
  REQUIRE(*first == 1);
}
