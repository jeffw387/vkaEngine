#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_set_layout.hpp"
#include "device.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "platform_glfw.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Create a layout") {
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

  std::unique_ptr<descriptor_set_layout> layoutPtr = {};
  descriptor_set_layout_builder{}
      .uniform_buffer(0, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{layoutPtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(layoutPtr->operator VkDescriptorSetLayout() != VK_NULL_HANDLE);
}