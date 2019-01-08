#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "instance.hpp"
#include "surface.hpp"

using namespace platform;
TEST_CASE("Create Surface") {
  auto instanceBuild = vka::instance_builder{};
  for (auto extension : glfw::get_required_instance_extensions()) {
    instanceBuild.add_extension(extension);
  }
  if (auto instanceResult = instanceBuild.build()) {
    auto instance = std::move(instanceResult.value());
    auto surfaceResult = vka::surface_builder{}
                             .width(100)
                             .height(100)
                             .title("test title")
                             .build(*instance);
    REQUIRE(surfaceResult);
  }
}