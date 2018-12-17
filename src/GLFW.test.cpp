#include "GLFW.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Init/Create Window") {
  SECTION("No exception during window creation.") {
    REQUIRE_NOTHROW(platform::GLFW::createWindow(100, 100, "test"));
  }

  SECTION("Valid window handle returned") {
    auto window = platform::GLFW::createWindow(100, 100, "test");
    REQUIRE(window != nullptr);
  }
}

TEST_CASE("Load global vulkan function") {
  auto result = platform::GLFW::loadVulkanFunction<PFN_vkCreateInstance>(
      VK_NULL_HANDLE, "vkCreateInstance");
  REQUIRE(result != nullptr);
}

TEST_CASE("Get required vulkan instance extensions") {
  auto extensions = platform::GLFW::getRequiredInstanceExtensions();
  REQUIRE(extensions.size() > 0);
}