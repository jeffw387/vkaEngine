#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_pool.hpp"
#include "GLFW.hpp"
#include "vka_instance.hpp"

TEST_CASE("Create descriptor pool") {
  using namespace platform;
  GLFW::init();
}