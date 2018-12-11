#include <string_view>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "Logger.hpp"

namespace GLFW {

struct Window {
  GLFWwindow* handle;
  VkSurfaceKHR surface;
};

struct Context {
  using WindowType = Window;
  
  Context() {
    MultiLogger::get()->info("Initializing GLFW");
    auto initResult = glfwInit();
    glfwSetErrorCallback([](int error, const char* desc) {
      MultiLogger::get()->error("GLFW error {}: {}", error, desc);
    });
  }

  template <typename T>
  T loadVulkanFunction(VkInstance instance, std::string_view functionName) {
    return reinterpret_cast<T>(glfwGetInstanceProcAddress(instance, functionName.data()));
  }

  template <typename T>
  T loadVulkanFunction(std::string_view functionName) {
    return reinterpret_cast<T>(glfwGetProcAddress(functionName.data()));
  }


  WindowType createSurface(VkInstance instance, int width, int height, std::string_view windowTitle) {
    Window window{};
    window.handle = glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr);
    auto surfaceResult = glfwCreateWindowSurface(instance, window.handle, nullptr, &window.surface);
  }

  ~Context() {
    MultiLogger::get()->info("Terminating GLFW");
    glfwTerminate();
  }
};
}