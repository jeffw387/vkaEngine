#include "Instance.hpp"
#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>
#include "GLFW.hpp"

using namespace GLFW;

TEST_CASE("Create Instance") {
  auto instance = std::make_unique<vka::Instance<GLFW::Context>>({
      "test app name",
      Version{0, 0, 0},
      std::vector<const char*>{},
      std::vector<const char*>{}});
  REQUIRE(instance->)
}