#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
// #include "platform_glfw.hpp"

using namespace vka;
TEST_CASE("Create a device") {
  auto instanceResult = instance_builder{}.build();
  auto physicalDeviceResult =
      physical_device_selector{}.select(**instanceResult);
  auto deviceResult = device_builder{}
                          .physical_device(**physicalDeviceResult)
                          .build(**instanceResult);
  REQUIRE(deviceResult);
}