#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_pool.hpp"
#include "platform_glfw.hpp"
#include "instance.hpp"

TEST_CASE("Create descriptor pool") {
  using namespace platform;
  glfw::init();
}