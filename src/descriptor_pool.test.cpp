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

  auto queueFamilyResult =
      queue_family_builder{}.graphics_support().queue(1.f).build(
          **physicalDeviceResult);
  REQUIRE(queueFamilyResult);
  auto deviceResult = device_builder{}
                          .add_queue_family(*queueFamilyResult)
                          .physical_device(**physicalDeviceResult)
                          .extension(swapchain_extension)
                          .build(**instanceResult);
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