#include "Instance.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include <vector>
#include "GLFW.hpp"
#include "Device.hpp"

using namespace platform;

TEST_CASE("Create Instance") {
  auto instance = std::make_unique<vka::Instance<GLFW>>(
      vka::InstanceCreateInfo{"test app name",
                              Version{0, 0, 0},
                              std::vector<const char*>{},
                              std::vector<const char*>{}});

  REQUIRE(instance->operator VkInstance() != VK_NULL_HANDLE);
}

TEST_CASE("Create Surface") {
  auto instance = std::make_unique<vka::Instance<GLFW>>(
      vka::InstanceCreateInfo{"test app name",
                              Version{0, 0, 0},
                              std::vector<const char*>{},
                              std::vector<const char*>{}});
  auto surface = instance->createSurface(
      vka::SurfaceCreateInfo{100, 100, "test window title"});

  REQUIRE(surface->operator VkSurfaceKHR() != VK_NULL_HANDLE);
}

TEST_CASE("Create Device") {
  auto instance = std::make_unique<vka::Instance<GLFW>>(
      vka::InstanceCreateInfo{"test app name",
                              Version{0, 0, 0},
                              std::vector<const char*>{},
                              std::vector<const char*>{}});
  auto surface = instance->createSurface(
      vka::SurfaceCreateInfo{100, 100, "test window title"});

  SECTION("Ensure no exceptions during construction") {
    REQUIRE_NOTHROW(instance->createDevice(
        surface.get(),
        std::vector<const char*>{},
        std::vector<vka::PhysicalDeviceFeatures>{},
        [](auto deviceInfo) { return deviceInfo.physicalDevices.at(0); }));
  }
  SECTION("Ensure valid device handle is returned") {
    auto device = instance->createDevice(
        surface.get(),
        std::vector<const char*>{},
        std::vector<vka::PhysicalDeviceFeatures>{},
        [](auto deviceInfo) { return deviceInfo.physicalDevices.at(0); });
    REQUIRE(device->operator VkDevice() != VK_NULL_HANDLE);
  }
}