#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include "instance.hpp"
#include "platform_glfw.hpp"

TEST_CASE("Create an instance") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr;
  auto instanceResult =
      vka::instance_builder{}.set_api_version(1, 0, 0).build();
  REQUIRE(instanceResult);
  instancePtr = std::move(instanceResult.value());
  REQUIRE(instancePtr->operator VkInstance() != VK_NULL_HANDLE);
}