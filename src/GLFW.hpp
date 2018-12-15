#include <string_view>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "gsl-lite.hpp"
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
  using KeyCallback = GLFWkeyfun;
  using MouseButtonCallback = GLFWmousebuttonfun;
  using CursorCallback = GLFWcursorposfun;

  template <typename T>
  static T loadVulkanFunction(VkInstance instance, std::string_view functionName) {
    init();
    return reinterpret_cast<T>(glfwGetInstanceProcAddress(instance, functionName.data()));
  }

  static WindowType* createWindow(int width, int height, std::string_view windowTitle);

  static VkSurfaceKHR createSurface(VkInstance instance, WindowType* window);

  static void setKeyCallback(WindowType* window, KeyCallback callback);

  static void setMouseButtonCallback(WindowType* window, MouseButtonCallback callback);
  
  static void setCursorCallback(WindowType* window, CursorCallback callback);

  static bool pollOS(WindowType* window);

  static gsl::span<const char*> getRequiredInstanceExtensions() {
    init();
    uint32_t count{};
    const char** ptr = glfwGetRequiredInstanceExtensions(&count);
    return gsl::span<const char*>{ptr, count};
  }

private:
  static std::unique_ptr<detail::GLFWOwner> glfwOwner;
  static void init();
};
}