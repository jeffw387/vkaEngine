#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "GLFW.hpp"

using namespace vka;
TEST_CASE("Create a device") {
  if (auto instance_result = instance_builder{}.build()) {
    auto device_result = device_builder{}.build(**instance_result);
    REQUIRE(device_result);
  } else {
    REQUIRE(false);
  }
}