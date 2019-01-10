#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_pool.hpp"
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"

using namespace platform;
using namespace vka;
TEST_CASE("Create descriptor pool") {
  glfw::init();
  std::unique_ptr<instance> instancePtr = {};
  auto instanceBuilder = instance_builder{}.add_layer(standard_validation);
  auto surfaceExtensions = platform::glfw::get_required_instance_extensions();
  for (auto extension : surfaceExtensions) {
    instanceBuilder.add_extension(extension);
  }
  instanceBuilder.build()
    .map([&d = instancePtr](auto value){ d = std::move(value); })
    .map_error([](auto error){ REQUIRE(false); });
  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}.select(*instancePtr)
    .map([&d = physicalDevice](auto value) { REQUIRE_NOTHROW(d = std::move(value.value())); })
    .map_error([](auto error) { REQUIRE(false); });

  queue_family graphicsPresentFamily = {};
      queue_family_builder{}.graphics_support().queue(1.f).build(
          physicalDevice);
  REQUIRE(queueFamilyResult);
  auto deviceResult = device_builder{}
                          .add_queue_family(*queueFamilyResult)
                          .physical_device(physicalDevice)
                          .extension(swapchain_extension)
                          .build(*instancePtr);
  REQUIRE(deviceResult);
  auto descriptorPoolResult =
      descriptor_pool_builder{}
          .add_type({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1})
          .max_sets(1)
          .build(**deviceResult);
  REQUIRE(descriptorPoolResult);
  REQUIRE(*descriptorPoolResult);
  REQUIRE(
      (**descriptorPoolResult).operator VkDescriptorPool() != VK_NULL_HANDLE);
}