#include "queue.hpp"

#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "move_into.hpp"
#include "physical_device.hpp"
#include "platform_glfw.hpp"
#include "queue_family.hpp"

using namespace vka;
TEST_CASE("Get queue from device") {
  platform::glfw::init();
  std::unique_ptr<instance> instancePtr = {};
  instance_builder{}
      .add_layer(standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { REQUIRE(false); });

  queue_family queueFamily = {};
  queue_family_builder{}
      .graphics_support()
      .queue(1.f)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<device> devicePtr = {};
  device_builder{}
      .add_queue_family(queueFamily)
      .physical_device(physicalDevice)
      .build(*instancePtr)
      .map(move_into{devicePtr})
      .map_error([](auto error) { REQUIRE(false); });

  queue queueHandle;
  queue_builder{}
      .queue_info(queueFamily, 0)
      .build(*devicePtr)
      .map(move_into{queueHandle})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(queueHandle.operator VkQueue() != VK_NULL_HANDLE);
}