#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "instance.hpp"
#include "GLFW.hpp"

TEST_CASE("Create an instance") {
  platform::GLFW::init();
  auto instance_result =
      vka::instance_builder{}.set_api_version(1, 0, 0).build();
  REQUIRE(std::get<VkResult>(instance_result) == VK_SUCCESS);
  REQUIRE(
      static_cast<VkInstance>(*std::get<0>(instance_result)) != VK_NULL_HANDLE);
}

TEST_CASE("Create surface") {
  auto window = platform::GLFW::createWindow(100, 100, "window title");
  auto glfwExtensions = platform::GLFW::getRequiredInstanceExtensions();
  auto builder = vka::instance_builder{};
  builder.set_api_version(1, 0, 0);
  for (auto extension : glfwExtensions) {
    builder.add_extension(extension);
  }
  auto instance_result = builder.build();
  auto& instance = instance_result.first;
  auto surface = platform::GLFW::createSurface(*instance, window);
  REQUIRE(surface != VK_NULL_HANDLE);
}