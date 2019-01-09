#include "input.hpp"

using namespace platform;
using namespace input;

static void key_callback(
    vka::WindowType* window,
    int keycode,
    int scancode,
    int action,
    int mods) {
  auto inputManager = glfw::get_user_pointer<manager>(window);
  inputManager->enqueue(event<key>{{keycode, action}, vka::Clock::now()});
}

static void mouse_button_callback(
    vka::WindowType* window,
    int button,
    int action,
    int mods) {
  auto inputManager = glfw::get_user_pointer<manager>(window);
  inputManager->enqueue(event<mouse>{{button, action}, vka::Clock::now()});
}

static void
cursor_position_callback(vka::WindowType* window, double x, double y) {
  auto inputManager = glfw::get_user_pointer<manager>(window);
  inputManager->update_cursor_position({x, y});
}

input::manager::manager(vka::WindowType* window) {
  glfw::set_user_pointer(window, this);
  glfw::set_key_callback(window, key_callback);
  glfw::set_mouse_button_callback(window, mouse_button_callback);
  glfw::set_cursor_callback(window, cursor_position_callback);
}