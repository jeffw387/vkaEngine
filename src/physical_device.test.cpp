#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "physical_device.hpp"
#include "instance.hpp"

using namespace vka;
TEST_CASE("Select first physical device") {
  auto instanceResult = instance_builder{}.build();
  REQUIRE(instanceResult);
  auto physicalDeviceResult =
      physical_device_selector{}.select(**instanceResult);
  REQUIRE(physicalDeviceResult);
  REQUIRE(physicalDeviceResult.value());
  REQUIRE(**physicalDeviceResult != VK_NULL_HANDLE);
}