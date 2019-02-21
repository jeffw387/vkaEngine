#include <catch2/catch.hpp>
#include <variant>
#include "variant_helper.hpp"

TEST_CASE(
    "Check that the actual size of currently held variant "
    "is returned") {
  using var_type =
      std::variant<uint8_t, uint16_t, uint32_t, uint64_t>;
  var_type v8 = uint8_t{};
  var_type v16 = uint16_t{};
  var_type v32 = uint32_t{};
  var_type v64 = uint64_t{};

  REQUIRE(std::visit(variant_size, v8) == sizeof(uint8_t));
  REQUIRE(
      std::visit(variant_size, v16) == sizeof(uint16_t));
  REQUIRE(
      std::visit(variant_size, v32) == sizeof(uint32_t));
  REQUIRE(
      std::visit(variant_size, v64) == sizeof(uint64_t));
}