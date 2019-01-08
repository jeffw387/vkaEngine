#include "GLFW.hpp"

namespace platform {
std::unique_ptr<detail::GLFWOwner> GLFW::glfwOwner = {};
void GLFW::init() {
  if (!glfwOwner) {
    glfwOwner = std::make_unique<detail::GLFWOwner>();
  }
}

tl::expected<GLFW::WindowType*, WindowCreateFailure>
GLFW::createWindow(int width, int height, std::string_view windowTitle) {
  init();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto result =
      glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr);
  if (result != nullptr) {
    return result;
  }
  return tl::make_unexpected(WindowCreateFailure{});
}

void GLFW::destroyWindow(GLFW::WindowType* window) {
  glfwDestroyWindow(window);
}

tl::expected<VkSurfaceKHR, VkResult> GLFW::createSurface(
    VkInstance instance,
    WindowType* window) {
  init();
  VkSurfaceKHR surface{};
  auto surface_result =
      glfwCreateWindowSurface(instance, window, nullptr, &surface);
  if (surface_result != VK_SUCCESS) {
    return tl::unexpected(surface_result);
  }
  return surface;
}

void GLFW::setKeyCallback(WindowType* window, GLFW::KeyCallback callback) {
  glfwSetKeyCallback(window, callback);
}

void GLFW::setMouseButtonCallback(
    WindowType* window,
    GLFW::MouseButtonCallback callback) {
  glfwSetMouseButtonCallback(window, callback);
}

void GLFW::setCursorCallback(
    WindowType* window,
    GLFW::CursorCallback callback) {
  glfwSetCursorPosCallback(window, callback);
}

window_should_close GLFW::pollOS(GLFW::WindowType* window) {
  init();
  glfwPollEvents();
  return {glfwWindowShouldClose(window) != 0};
}
}  // namespace platform