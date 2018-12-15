#include "GLFW.hpp"

namespace GLFW {
std::unique_ptr<detail::GLFWOwner> Context::glfwOwner = {};
void Context::init() {
  if (!glfwOwner) {
    glfwOwner = std::make_unique<detail::GLFWOwner>();
  }
}

Context::WindowType*
Context::createWindow(int width, int height, std::string_view windowTitle) {
  init();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  return glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr);
}

VkSurfaceKHR Context::createSurface(VkInstance instance, WindowType* window) {
  init();
  VkSurfaceKHR result{};
  auto surfaceResult =
      glfwCreateWindowSurface(instance, window, nullptr, &result);
  return result;
}

void Context::setKeyCallback(
    WindowType* window,
    Context::KeyCallback callback) {
  glfwSetKeyCallback(window, callback);
}

void Context::setMouseButtonCallback(
    WindowType* window,
    Context::MouseButtonCallback callback) {
  glfwSetMouseButtonCallback(window, callback);
}

void Context::setCursorCallback(
    WindowType* window,
    Context::CursorCallback callback) {
  glfwSetCursorPosCallback(window, callback);
}

bool Context::pollOS(Context::WindowType* window) {
  init();
  glfwPollEvents();
  return glfwWindowShouldClose(window);
}
}  // namespace GLFW