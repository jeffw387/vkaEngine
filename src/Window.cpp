#include "Window.hpp"
#include <GLFW/glfw3.h>

namespace vka {
Window::Window(int width, int height, const char* windowTitle) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NO_API);
  window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
}
Window::~Window() { glfwDestroyWindow(window); }
}  // namespace vka
