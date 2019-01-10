#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include "physical_device.hpp"
#include "instance.hpp"

using namespace vka;
TEST_CASE("Select first physical device") {
  std::unique_ptr<instance> instancePtr = {};
  instance_builder{}
      .build()
      .map([& d = instancePtr](auto value) { d = std::move(value); })
      .map_error([](auto error) { REQUIRE(false); });
  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map([& d = physicalDevice](auto value) {
        value.map([& d = d](auto value) { d = value; }).or_else([] {
          REQUIRE(false);
        });
      })
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(physicalDevice != VK_NULL_HANDLE);
}