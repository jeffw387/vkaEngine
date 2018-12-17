#include "GLFW.hpp"

namespace platform {
std::unique_ptr<detail::GLFWOwner> GLFW::glfwOwner = {};
void GLFW::init() {
  if (!glfwOwner) {
    glfwOwner = std::make_unique<detail::GLFWOwner>();
  }
}

GLFW::WindowType*
GLFW::createWindow(int width, int height, std::string_view windowTitle) {
  init();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  return glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr);
}

VkSurfaceKHR GLFW::createSurface(VkInstance instance, WindowType* window) {
  init();
  VkSurfaceKHR result{};
  auto surfaceResult =
      glfwCreateWindowSurface(instance, window, nullptr, &result);
  return result;
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

bool GLFW::pollOS(GLFW::WindowType* window) {
  init();
  glfwPollEvents();
  return glfwWindowShouldClose(window);
}
}  // namespace platform