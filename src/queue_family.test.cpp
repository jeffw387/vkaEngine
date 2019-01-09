#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "queue_family.hpp"
#include "surface.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "platform_glfw.hpp"

using namespace vka;
TEST_CASE("Select a graphics/present queue") {
  auto instanceBuilder = instance_builder{};
  auto surfaceExtensions = platform::glfw::get_required_instance_extensions();
  for (auto extension : surfaceExtensions) {
    instanceBuilder.add_extension(extension);
  }
  auto instanceResult =
      instanceBuilder.add_layer("VK_LAYER_LUNARG_standard_validation").build();
  REQUIRE(instanceResult);
  auto physicalDeviceResult =
      physical_device_selector{}.select(**instanceResult);
  REQUIRE(physicalDeviceResult);
  REQUIRE(physicalDeviceResult.value());
  auto surfaceResult = surface_builder{}
                           .width(100)
                           .height(100)
                           .title("test window")
                           .build(**instanceResult);
  REQUIRE(surfaceResult);
  auto queueFamilyResult = queue_family_builder{}
                               .graphics_support()
                               .present_support()
                               .queue(1.f)
                               .build(**physicalDeviceResult, **surfaceResult);
  REQUIRE(queueFamilyResult);
}