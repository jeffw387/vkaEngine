#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "queue_family.hpp"
#include "surface.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "platform_glfw.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Select a graphics/present queue") {
  auto instanceBuilder = instance_builder{};
  auto surfaceExtensions = platform::glfw::get_required_instance_extensions();
  for (auto extension : surfaceExtensions) {
    instanceBuilder.add_extension(extension);
  }
  std::unique_ptr<instance> instancePtr = {};
  instanceBuilder.add_layer("VK_LAYER_LUNARG_standard_validation")
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { REQUIRE(false); });
  std::unique_ptr<surface> surfacePtr = {};
  surface_builder{}
                           .width(100)
                           .height(100)
                           .title("test window")
      .build(*instancePtr)
      .map(move_into{surfacePtr})
      .map_error([](auto error) { REQUIRE(false); });
  queue_family queueFamily = {};
  queue_family_builder{}
                               .graphics_support()
      .present_support(*surfacePtr)
                               .queue(1.f)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error) { REQUIRE(false); });
}