#pragma once

#include <GLFW/glfw3.h>

namespace vka {

class Window {
public:
  Window(int width, int height, const char* windowTitle);
  ~Window();

  GLFWwindow* getHandle() { return window; }

private:
  GLFWwindow* window;
};
}  // namespace vka