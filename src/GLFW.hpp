#include <string_view>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <expected.hpp>
#include "gsl-lite.hpp"

namespace platform {

namespace detail {
struct GLFWOwner {
  GLFWOwner() {
    auto result = glfwInit();
    if (result != GLFW_TRUE) {
      throw result;
    }
  }
  GLFWOwner(const GLFWOwner&) = delete;
  GLFWOwner(GLFWOwner&&) = default;
  GLFWOwner& operator=(const GLFWOwner&) = delete;
  GLFWOwner& operator=(GLFWOwner&&) = default;

  ~GLFWOwner() { glfwTerminate(); }
};
}  // namespace detail
struct window_should_close {
  bool should_close = {};

  operator bool() const noexcept { return should_close; }
};

struct WindowCreateFailure {};

class GLFW {
public:
  using WindowType = GLFWwindow;
  using KeyCallback = GLFWkeyfun;
  using MouseButtonCallback = GLFWmousebuttonfun;
  using CursorCallback = GLFWcursorposfun;

  template <typename T>
  static T loadVulkanFunction(
      VkInstance instance,
      std::string_view functionName) {
    init();
    return reinterpret_cast<T>(
        glfwGetInstanceProcAddress(instance, functionName.data()));
  }

  static tl::expected<WindowType*, WindowCreateFailure>
  createWindow(int width, int height, std::string_view windowTitle);

  static void destroyWindow(WindowType* window);

  static tl::expected<VkSurfaceKHR, VkResult> createSurface(
      VkInstance instance,
      WindowType* window);

  static void setKeyCallback(WindowType* window, KeyCallback callback);

  static void setMouseButtonCallback(
      WindowType* window,
      MouseButtonCallback callback);

  static void setCursorCallback(WindowType* window, CursorCallback callback);

  static void setUserPointer(WindowType* window, void* userPointer) {
    glfwSetWindowUserPointer(window, userPointer);
  }

  template <typename T>
  static T* getUserPointer(WindowType* window) {
    return reinterpret_cast<T*>(glfwGetWindowUserPointer(window));
  }

  static window_should_close pollOS(WindowType* window);

  static gsl::span<const char*> getRequiredInstanceExtensions() {
    init();
    uint32_t count{};
    const char** ptr = glfwGetRequiredInstanceExtensions(&count);
    return gsl::span<const char*>{ptr, count};
  }

  static void init();

private:
  static std::unique_ptr<detail::GLFWOwner> glfwOwner;
};
}  // namespace platform