
#include <catch2/catch.hpp>
#include <memory>
#include "instance.hpp"
#include "platform_glfw.hpp"
#include "move_into.hpp"

TEST_CASE("Create an instance") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr;
  vka::instance_builder{}
      .set_api_version(1, 0, 0)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  REQUIRE(instancePtr->operator VkInstance() != VK_NULL_HANDLE);
}