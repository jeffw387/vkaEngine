#include "swapchain.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "instance.hpp"
#include "queue_family.hpp"
#include "physical_device.hpp"
#include "device.hpp"
#include "platform_glfw.hpp"
#include "surface.hpp"

using namespace vka;
TEST_CASE("Create a swapchain") {
  auto instanceBuilder = instance_builder{}.add_layer(standard_validation);
  auto surfaceExtensions = platform::glfw::get_required_instance_extensions();
  for (auto extension : surfaceExtensions) {
    instanceBuilder.add_extension(extension);
  }
  auto instanceResult = instanceBuilder.build();
  REQUIRE(instanceResult);
  auto physicalDeviceResult =
      physical_device_selector{}.select(**instanceResult);
  REQUIRE(physicalDeviceResult);
  auto surfaceResult = surface_builder{}
                           .width(100)
                           .height(100)
                           .title("test title")
                           .build(**instanceResult);
  REQUIRE(surfaceResult);
  auto queueFamilyResult = queue_family_builder{}
                               .graphics_support()
                               .present_support(**surfaceResult)
                               .queue(1.f)
                               .build(**physicalDeviceResult);
  REQUIRE(queueFamilyResult);
  auto deviceResult = device_builder{}
                          .add_queue_family(*queueFamilyResult)
                          .physical_device(**physicalDeviceResult)
                          .extension(swapchain_extension)
                          .build(**instanceResult);
  REQUIRE(deviceResult);
  auto swapchainResult =
      swapchain_builder{}
          .present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .queue_family_index(queueFamilyResult->familyIndex)
          .build(**physicalDeviceResult, **surfaceResult, **deviceResult);
  REQUIRE(swapchainResult);
  REQUIRE(*swapchainResult);
  REQUIRE((**swapchainResult != VK_NULL_HANDLE));
}