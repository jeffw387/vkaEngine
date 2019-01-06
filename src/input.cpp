#include "input.hpp"

using namespace platform;
using namespace input;

static void key_callback(
    vka::WindowType* window,
    int keycode,
    int scancode,
    int action,
    int mods) {
  auto inputManager = GLFW::getUserPointer<manager>(window);
  inputManager->enqueue(event<key>{{keycode, action}, vka::Clock::now()});
}

static void mouse_button_callback(
    vka::WindowType* window,
    int button,
    int action,
    int mods) {
  auto inputManager = GLFW::getUserPointer<manager>(window);
  inputManager->enqueue(event<mouse>{{button, action}, vka::Clock::now()});
}

static void
cursor_position_callback(vka::WindowType* window, double x, double y) {
  auto inputManager = GLFW::getUserPointer<manager>(window);
  inputManager->update_cursor_position({x, y});
}

input::manager::manager(vka::WindowType* window) {
  GLFW::setUserPointer(window, this);
  GLFW::setKeyCallback(window, key_callback);
  GLFW::setMouseButtonCallback(window, mouse_button_callback);
  GLFW::setCursorCallback(window, cursor_position_callback);
}