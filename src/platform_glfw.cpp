#include "platform_glfw.hpp"

namespace platform {
std::unique_ptr<detail::glfw_owner> glfwOwner = {};
void glfw::init() {
  if (!glfwOwner) {
    glfwOwner = std::make_unique<detail::glfw_owner>();
  }
}

tl::expected<glfw::window_type*, window_create_failure>
glfw::create_window(
    int width,
    int height,
    std::string_view windowTitle) {
  init();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto result = glfwCreateWindow(
      width, height, windowTitle.data(), nullptr, nullptr);
  if (result != nullptr) {
    return result;
  }
  return tl::make_unexpected(window_create_failure{});
}

void glfw::destroy_window(glfw::window_type* window) {
  glfwDestroyWindow(window);
}

tl::expected<VkSurfaceKHR, VkResult> glfw::create_surface(
    VkInstance instance,
    window_type* window) {
  init();
  VkSurfaceKHR surface{};
  auto surfaceResult = glfwCreateWindowSurface(
      instance, window, nullptr, &surface);
  if (surfaceResult != VK_SUCCESS) {
    return tl::unexpected(surfaceResult);
  }
  return surface;
}

void glfw::set_key_callback(
    window_type* window,
    glfw::key_callback callback) {
  glfwSetKeyCallback(window, callback);
}

void glfw::set_mouse_button_callback(
    window_type* window,
    glfw::mouse_button_callback callback) {
  glfwSetMouseButtonCallback(window, callback);
}

void glfw::set_cursor_callback(
    window_type* window,
    glfw::cursor_callback callback) {
  glfwSetCursorPosCallback(window, callback);
}

window_should_close glfw::poll_os(
    glfw::window_type* window) {
  init();
  glfwPollEvents();
  return {glfwWindowShouldClose(window) != 0};
}
}  // namespace platform