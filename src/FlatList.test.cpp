#include "FlatList.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("Adding item to queue") {
  FlatList<int, 3> queue;
  REQUIRE(queue.size() == 0);
  REQUIRE(queue.capacity() == 3);

  auto push_result = queue.push_last(1);
  REQUIRE(queue.size() == 1);
  REQUIRE(push_result == true);
}

TEST_CASE("Adding an item when at capacity should fail") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(1);
  push_result = queue.push_last(1);

  REQUIRE(push_result == true);

  push_result = queue.push_last(1);
  REQUIRE(push_result == false);
}

TEST_CASE("Read first item in queue with size 1") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  auto first = queue.first();

  REQUIRE(first);
  REQUIRE(*first == 1);
}

TEST_CASE("Read first item in queue with size 2") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(2);
  auto first = queue.first();

  REQUIRE(first);
  REQUIRE(first == 1);
}

TEST_CASE("Add a single item them pop it, leaving queue empty") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  queue.pop_first();

  REQUIRE(queue.size() == 0);
}

TEST_CASE("Add two items, pop the first, read the next") {
  FlatList<int, 3> queue;
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
  FlatList<
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

TEST_CASE("Add three items, get item behind first") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(2);
  push_result = queue.push_last(3);

  auto begin = queue.begin();
  ++begin;

  REQUIRE(*begin == 2);
}

TEST_CASE("Add two items, pop last in queue") {
  FlatList<int, 3> queue;
  auto push_result = queue.push_last(1);
  push_result = queue.push_last(2);

  auto last = queue.last();
  REQUIRE(last);
  REQUIRE(*last == 2);
  REQUIRE(queue.size() == 2);

  queue.pop_last();
  REQUIRE(queue.last());
  REQUIRE(*queue.last() == 1);
  REQUIRE(queue.size() == 1);
}

TEST_CASE("Wrap forward to beginning of storage") {
  FlatList<int, 2> list;
  list.push_last(1);
  REQUIRE(list.last_iterator().index == 0);
  list.push_last(2);
  REQUIRE(list.last_iterator().index == 1);
  list.pop_first();
  list.push_last(3);
  REQUIRE(list.last_iterator().index == 0);
}