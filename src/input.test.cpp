
#include <catch2/catch.hpp>
#include "input.hpp"
#include "instance.hpp"

TEST_CASE("Create input manager") {
  auto createLambda = [] {
    if (auto instanceResult =
            vka::instance_builder{}.build()) {
      auto instance = std::move(*instanceResult);
      if (auto surfaceResult = vka::surface_builder{}
                                   .width(100)
                                   .height(100)
                                   .title("test title")
                                   .build(*instance)) {
        auto surface = std::move(*surfaceResult);
        auto inputManager = input::manager{*surface};
      }
    }
  };
  REQUIRE_NOTHROW(createLambda());
}

TEST_CASE(
    "Simulate key strokes, get events before arbitrary "
    "time") {
  if (auto instanceResult =
          vka::instance_builder{}.build()) {
    auto instance = std::move(*instanceResult);
    if (auto surfaceResult = vka::surface_builder{}
                                 .width(100)
                                 .height(100)
                                 .title("test title")
                                 .build(*instance)) {
      auto surface = std::move(*surfaceResult);
      auto inputManager = input::manager{*surface};
      for (int i = {}; i < 3; ++i) {
        inputManager.enqueue(input::event<input::key>{
            {GLFW_KEY_0, GLFW_PRESS}, vka::Clock::now()});
      }

      for (int i = {}; i < 3; ++i) {
        auto eventOptional = inputManager.next_event_before(
            vka::Clock::now());
        REQUIRE(eventOptional);
        std::visit(
            [](auto event_variant) {
              REQUIRE(
                  event_variant.eventSignature.code ==
                  GLFW_KEY_0);
              REQUIRE(
                  event_variant.eventSignature.action ==
                  GLFW_PRESS);
            },
            *eventOptional);
      }
      SECTION(
          "No events remain, optional should be empty") {
        auto eventOptional = inputManager.next_event_before(
            vka::Clock::now());
        REQUIRE(!eventOptional);
      }
    }
  }
}