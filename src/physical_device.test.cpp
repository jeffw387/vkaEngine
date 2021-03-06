
#include <catch2/catch.hpp>
#include <memory>
#include "instance.hpp"
#include "move_into.hpp"
#include "physical_device.hpp"
#include "platform_glfw.hpp"

using namespace vka;
TEST_CASE("Select first physical device") {
  platform::glfw::init();
  std::unique_ptr<instance> instancePtr = {};
  instance_builder{}
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });
  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(physicalDevice != VK_NULL_HANDLE);
}