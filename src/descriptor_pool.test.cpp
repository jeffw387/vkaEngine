#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_pool.hpp"
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "move_into.hpp"

using namespace platform;
using namespace vka;
TEST_CASE("Create descriptor pool") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr = {};
  vka::instance_builder{}
      .add_layer(vka::standard_validation)
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

  SECTION("Add types individually") {
    std::unique_ptr<descriptor_pool> descriptorPoolPtr = {};
    descriptor_pool_builder{}
        .add_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .max_sets(1)
        .build(*devicePtr)
        .map(move_into{descriptorPoolPtr})
        .map_error([](auto error) { REQUIRE(false); });
    REQUIRE(descriptorPoolPtr->operator VkDescriptorPool() != VK_NULL_HANDLE);
  }

  SECTION("Add types from a layout") {
    std::unique_ptr<descriptor_set_layout> layoutPtr = {};
    descriptor_set_layout_builder{}
        .image(0, 3, VK_SHADER_STAGE_FRAGMENT_BIT)
        .sampler(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT, {})
        .uniform_buffer(2, 1, VK_SHADER_STAGE_VERTEX_BIT)
        .uniform_buffer(3, 2, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build(*devicePtr)
        .map(move_into{layoutPtr})
        .map_error([](auto error) { REQUIRE(false); });

    std::unique_ptr<descriptor_pool> descriptorPoolPtr = {};
    descriptor_pool_builder{}
        .add_layout(layoutPtr.get(), 3)
        .build(*devicePtr)
        .map(move_into{descriptorPoolPtr})
        .map_error([](auto error) { REQUIRE(false); });
    REQUIRE(descriptorPoolPtr->operator VkDescriptorPool() != VK_NULL_HANDLE);
  }
}