#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include "instance.hpp"
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