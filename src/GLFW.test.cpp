#include "GLFW.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Init") {
  try {
    GLFW::Context::createWindow(100, 100, "test");
  }
  catch(const std::exception& e) {
    REQUIRE_FALSE(true);      
  }
}