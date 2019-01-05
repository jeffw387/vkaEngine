#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include "vka_instance.hpp"
#include "GLFW.hpp"

TEST_CASE("Create an instance") {
  platform::GLFW::init();
  std::unique_ptr<vka::instance> instance_ptr;
  auto instance_result =
      vka::instance_builder{}.set_api_version(1, 0, 0).build();
  REQUIRE(instance_result);
  instance_ptr = std::move(instance_result.value());
  REQUIRE(instance_ptr->operator VkInstance() != VK_NULL_HANDLE);
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
  auto& instance = instance_result.value();
  auto surface = platform::GLFW::createSurface(*instance, window.value());
  REQUIRE(surface);
  REQUIRE(surface.value() != VK_NULL_HANDLE);
}