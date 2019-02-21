#include "swapchain.hpp"

#include <catch2/catch.hpp>
#include "device.hpp"
#include "instance.hpp"
#include "move_into.hpp"
#include "physical_device.hpp"
#include "platform_glfw.hpp"
#include "queue_family.hpp"
#include "surface.hpp"

using namespace vka;
TEST_CASE("Create a swapchain") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr = {};
  vka::instance_builder{}
      .add_extensions(
          platform::glfw::
              get_required_instance_extensions())
      .add_layer(vka::standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<vka::surface> surfacePtr = {};
  vka::surface_builder{}
      .width(100)
      .height(100)
      .title("test title")
      .build(*instancePtr)
      .map(move_into{surfacePtr})
      .map_error([](auto error) { REQUIRE(false); });

  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { REQUIRE(false); });

  queue_family queueFamily = {};
  queue_family_builder{}
      .graphics_support()
      .present_support(*surfacePtr)
      .queue(1.f)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<device> devicePtr = {};
  device_builder{}
      .add_queue_family(queueFamily)
      .physical_device(physicalDevice)
      .extension(swapchain_extension)
      .build(*instancePtr)
      .map(move_into{devicePtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<swapchain> swapchainPtr = {};
  swapchain_builder{}
      .present_mode(VK_PRESENT_MODE_FIFO_KHR)
      .queue_family_index(queueFamily.familyIndex)
      .build(physicalDevice, *surfacePtr, *devicePtr)
      .map(move_into{swapchainPtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(
      swapchainPtr->operator VkSwapchainKHR() !=
      VK_NULL_HANDLE);
}