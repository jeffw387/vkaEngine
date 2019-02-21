
#include <catch2/catch.hpp>
#include "instance.hpp"
#include "move_into.hpp"
#include "platform_glfw.hpp"
#include "surface.hpp"

using namespace platform;
TEST_CASE("Create Surface") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr = {};
  vka::instance_builder{}
      .add_extensions(
          glfw::get_required_instance_extensions())
      .add_layer(vka::standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<vka::surface> surfacePtr = {};
  vka::surface_builder{}
      .width(100)
      .height(100)
      .title("test title")
      .build(*instancePtr)
      .map(move_into{surfacePtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(
      surfacePtr->operator vka::WindowType *() != nullptr);
  REQUIRE(
      surfacePtr->operator VkSurfaceKHR() !=
      VK_NULL_HANDLE);
}