#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"

using namespace vka;
TEST_CASE("Create a device") {
  auto instanceResult =
      instance_builder{}.add_layer(standard_validation).build();
  REQUIRE(instanceResult);
  auto physicalDeviceResult =
      physical_device_selector{}.select(**instanceResult);
  REQUIRE(physicalDeviceResult);
  auto graphicsQueueFamily =
      queue_family_builder{}.graphics_support().queue(1.f).build(
          **physicalDeviceResult);
  REQUIRE(graphicsQueueFamily);
  auto deviceResult = device_builder{}
                          .physical_device(**physicalDeviceResult)
                          .add_queue_family(*graphicsQueueFamily)
                          .build(**instanceResult);
  REQUIRE(deviceResult);
  REQUIRE(*deviceResult);
  REQUIRE((**deviceResult).operator VkDevice() != VK_NULL_HANDLE);
}