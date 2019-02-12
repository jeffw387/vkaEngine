#include "platform_glfw.hpp"
#include "logger.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Init/Create Window") {
  SECTION("No exception during window creation.") {
    REQUIRE(platform::glfw::create_window(100, 100, "test"));
  }

  SECTION("Valid window handle returned") {
    auto window = platform::glfw::create_window(100, 100, "test");
    REQUIRE(window.value() != nullptr);
  }
}

TEST_CASE("Load global vulkan function") {
  auto result = platform::glfw::load_vulkan_function<PFN_vkCreateInstance>(
      VK_NULL_HANDLE, "vkCreateInstance");
  REQUIRE(result != nullptr);
}

TEST_CASE("Get required vulkan instance extensions") {
  auto vulkanSupport = glfwVulkanSupported();
  REQUIRE(vulkanSupport);
  
  auto extensions = platform::glfw::get_required_instance_extensions();
  REQUIRE(extensions.size() > 0);
}