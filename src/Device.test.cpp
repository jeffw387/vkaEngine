#include "GLFW.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Create device-dependent objects") {
  auto instance = std::make_unique<vka::Instance<GLFW::Context>>(
      vka::InstanceCreateInfo{"test app name",
                              Version{0, 0, 0},
                              std::vector<const char*>{},
                              std::vector<const char*>{}});
  auto surface = instance->createSurface(
      vka::SurfaceCreateInfo{100, 100, "test window title"});
  auto device = instance->createDevice(
      surface.get(),
      std::vector<const char*>{},
      std::vector<vka::PhysicalDeviceFeatures>{},
      [](auto deviceInfo) { return deviceInfo.physicalDevices.at(0); });

  SECTION("Create buffer") {
    auto buffer =
        device->createBuffer(1024, 0, VMA_MEMORY_USAGE_CPU_ONLY, true);

    REQUIRE(buffer->operator VkBuffer() != VK_NULL_HANDLE);
    REQUIRE(buffer->operator VmaAllocation() != nullptr);
  }

  SECTION("Create Image2D") {
    auto image = device->createImage2D(
        VkExtent2D{100, 100},
        VK_FORMAT_R8G8B8A8_UNORM,
        0,
        vka::ImageAspect::Color,
        true);
  }
}