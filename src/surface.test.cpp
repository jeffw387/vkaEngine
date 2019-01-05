#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "instance.hpp"
#include "surface.hpp"

using namespace platform;
TEST_CASE("Create Surface") {
  auto instance_build = vka::instance_builder{};
  for (auto extension : GLFW::getRequiredInstanceExtensions()) {
    instance_build.add_extension(extension);
  }
  if (auto instance_result = instance_build.build()) {
    auto instance = std::move(instance_result.value());
    auto surface_result = vka::surface_builder{}
                              .width(100)
                              .height(100)
                              .title("test title")
                              .build(*instance);
    REQUIRE(surface_result);
  }
}