#include <string_view>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "Logger.hpp"

namespace GLFW {

namespace detail {
struct GLFWOwner {
  GLFWOwner() {
    auto result = glfwInit();
    if (result != GLFW_TRUE) {
      throw result;
    }
  }

  ~GLFWOwner() {
    glfwTerminate();
  }
};
}

class Context {
public:
  using WindowType = GLFWwindow;

  template <typename T>
  static T loadVulkanFunction(VkInstance instance, std::string_view functionName) {
    init();
    return reinterpret_cast<T>(glfwGetInstanceProcAddress(instance, functionName.data()));
  }

  template <typename T>
  static T loadVulkanFunction(std::string_view functionName) {
    init();
    return reinterpret_cast<T>(glfwGetProcAddress(functionName.data()));
  }

  static WindowType* createWindow(int width, int height, std::string_view windowTitle);

  static VkSurfaceKHR createSurface(VkInstance instance, WindowType* window);

private:
  static std::unique_ptr<detail::GLFWOwner> glfwOwner;
  static void init();
};
}