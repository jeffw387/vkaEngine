#include "command_buffer.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "platform_glfw.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Allocate a command buffer") {
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

  std::unique_ptr<command_pool> commandPoolPtr = {};
  command_pool_builder{}
      .queue_family_index(queueFamily.familyIndex)
      .build(*devicePtr)
      .map(move_into{commandPoolPtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<command_buffer> commandPtr = {};
  command_buffer_allocator{}
      .set_command_pool(commandPoolPtr.get())
      .allocate(*devicePtr)
      .map(move_into{commandPtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(commandPtr->operator VkCommandBuffer() != VK_NULL_HANDLE);
}