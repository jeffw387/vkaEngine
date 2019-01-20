#include "ObjectPool.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Allocate an object") {
  Pool<int, 3> pool = {};
  auto obj = pool.allocate();
  REQUIRE(obj);
  REQUIRE(obj.value() == int{});
}

TEST_CASE("An allocated object isn't necessarily in initial state") {
  Pool<int, 1> pool = {};
  auto obj = pool.allocate();
  REQUIRE(obj.value() == int{});
  obj.value() = 2;
  pool.free(obj);

  auto obj2 = pool.allocate();
  REQUIRE(obj2);
  REQUIRE(obj2.value() == 2);
}

TEST_CASE("Over-allocation results in null pooled objects") {
  Pool<int, 1> pool = {};
  auto obj = pool.allocate();
  auto obj2 = pool.allocate();

  REQUIRE(obj2 == false);
}

TEST_CASE("Dereference object and assign") {
  Pool<int, 1> pool;
  auto obj = pool.allocate();
  REQUIRE_NOTHROW(obj.value() = 1);
}

TEST_CASE("Returning null pooled object to empty pool doesn't free anything") {
  Pool<int, 1> pool = {};
  auto obj = pool.allocate();
  auto obj2 = pool.allocate();
  REQUIRE(!obj2);

  pool.free(obj2);
  auto obj3 = pool.allocate();

  REQUIRE(!obj3);
}

TEST_CASE("Default constructor default-inits array") {
  Pool<int, 1> pool0 = {};
  Pool<int, 1> pool1;

  auto obj0 = pool0.allocate();
  auto obj1 = pool1.allocate();

  REQUIRE(obj0.value() == int{});
  REQUIRE(obj1.value() == int{});
}