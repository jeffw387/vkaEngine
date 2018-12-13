#include "GLFW.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Init/Create Window") {
  SECTION("No exception during window creation.") {
    REQUIRE_NOTHROW (
      GLFW::Context::createWindow(100, 100, "test")
    );
  }
  
  SECTION("Valid window handle returned") {
    auto window = GLFW::Context::createWindow(100, 100, "test");
    REQUIRE(window != nullptr);
  }
}

TEST_CASE("Load global vulkan function") {
  auto result = GLFW::Context::loadVulkanFunction<PFN_vkCreateInstance>(VK_NULL_HANDLE, "vkCreateInstance");
  REQUIRE(result != nullptr);
}

TEST_CASE("Get required vulkan instance extensions") {
  auto extensions = GLFW::Context::getRequiredInstanceExtensions();
  REQUIRE(extensions.size() > 0);
}

// bool resizeCallbackCalled = false;
// void resizeCallback(GLFWwindow* window, int width, int height) {
//   resizeCallbackCalled = true;
// }
//
// Not sure how to trigger an event programmatically to fulfill this test
// TEST_CASE("Poll for OS events") {
//   auto window = GLFW::Context::createWindow(100, 100, "test");
//   glfwSetWindowSizeCallback(window, resizeCallback);
//   glfwSetWindowSize(window, 200, 200);
//
//   REQUIRE(resizeCallbackCalled == false);
//   auto shouldClose = GLFW::Context::pollOS(window);
//   REQUIRE(shouldClose == false);
//   REQUIRE(resizeCallbackCalled == true);
// }