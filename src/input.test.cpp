#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "input.hpp"
#include "instance.hpp"

TEST_CASE("Create input manager") {
  auto create_lambda = [] {
    if (auto instance_result = vka::instance_builder{}.build()) {
      auto instance = std::move(*instance_result);
      if (auto surface_result = vka::surface_builder{}
                                    .width(100)
                                    .height(100)
                                    .title("test title")
                                    .build(*instance)) {
        auto surface = std::move(*surface_result);
        auto inputManager = input::manager{*surface};
      }
    }
  };
  REQUIRE_NOTHROW(create_lambda());
}

TEST_CASE("Simulate key strokes, get events before arbitrary time") {
  if (auto instance_result = vka::instance_builder{}.build()) {
    auto instance = std::move(*instance_result);
    if (auto surface_result = vka::surface_builder{}
                                  .width(100)
                                  .height(100)
                                  .title("test title")
                                  .build(*instance)) {
      auto surface = std::move(*surface_result);
      auto inputManager = input::manager{*surface};
      for (int i = {}; i < 3; ++i) {
        inputManager.enqueue(input::event<input::key>{{GLFW_KEY_0, GLFW_PRESS},
                                                      vka::Clock::now()});
      }

      for (int i = {}; i < 3; ++i) {
        auto event_optional = inputManager.next_event_before(vka::Clock::now());
        REQUIRE(event_optional);
        std::visit(
            [](auto event_variant) {
              REQUIRE(event_variant.eventSignature.code == GLFW_KEY_0);
              REQUIRE(event_variant.eventSignature.action == GLFW_PRESS);
            },
            *event_optional);
      }
      SECTION("No events remain, optional should be empty") {
        auto event_optional = inputManager.next_event_before(vka::Clock::now());
        REQUIRE(!event_optional);
      }
    }
  }
}