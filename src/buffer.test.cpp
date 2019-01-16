#include "buffer.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "memory_allocator.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Create a 1024b vertex buffer (gpu-local)") {
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

  std::unique_ptr<allocator> allocatorPtr = {};
  allocator_builder{}
      .physical_device(physicalDevice)
      .device(*devicePtr)
      .build()
      .map(move_into{allocatorPtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<buffer> bufferPtr = {};
  buffer_builder{}
      .size(1024)
      .gpu_only()
      .vertex_buffer()
      .queue_family_index(queueFamily.familyIndex)
      .transfer_destination()
      .build(*allocatorPtr)
      .map(move_into{bufferPtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(bufferPtr->operator VkBuffer() != VK_NULL_HANDLE);
  REQUIRE(bufferPtr->operator VmaAllocation() != nullptr);
}