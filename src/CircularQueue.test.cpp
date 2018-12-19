#include "CircularQueue.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>

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

TEST_CASE("Read first item in queue with size 2") {
  CircularQueue<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(2);
  auto first = queue.first();

  REQUIRE(first);
  REQUIRE(first == 1);
}

TEST_CASE("Add a single item them pop it, leaving queue empty") {
  CircularQueue<int, 3> queue;
  auto push_result = queue.push_last(1);
  queue.pop_first();

  REQUIRE(queue.size() == 0);
}

TEST_CASE("Add two items, pop the first, read the next") {
  CircularQueue<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(2);

  REQUIRE(queue.size() == 2);
  REQUIRE(queue.first().value() == 1);

  queue.pop_first();
  auto next = queue.first();

  REQUIRE(next);
  REQUIRE(*next == 2);
  REQUIRE(queue.size() == 1);
}

template <typename T>
struct TestStructWithDestructor {
  TestStructWithDestructor(T func) : func{func} {}
  ~TestStructWithDestructor() { func(); }
  T func;
};

TEST_CASE("Item destructor is called on item pop") {
  bool destructorRun = false;
  auto reportDestruct = [&destructorRun]() { destructorRun = true; };
  CircularQueue<
      std::unique_ptr<TestStructWithDestructor<decltype(reportDestruct)>>,
      1>
      queue;
  queue.push_last(
      std::make_unique<TestStructWithDestructor<decltype(reportDestruct)>>(
          reportDestruct));

  REQUIRE(destructorRun == false);

  queue.pop_first();
  REQUIRE(destructorRun == true);
}
