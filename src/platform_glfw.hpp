#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <string_view>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <tl/expected.hpp>
#include "gsl-lite.hpp"

namespace platform {

namespace detail {
struct glfw_owner {
  glfw_owner() {
    auto result = glfwInit();
    if (result != GLFW_TRUE) {
      throw result;
    }
  }
  glfw_owner(const glfw_owner&) = delete;
  glfw_owner(glfw_owner&&) = default;
  glfw_owner& operator=(const glfw_owner&) = delete;
  glfw_owner& operator=(glfw_owner&&) = default;

  ~glfw_owner() { glfwTerminate(); }
};
}  // namespace detail
struct window_should_close {
  bool should_close = {};

  operator bool() const noexcept { return should_close; }
};

struct window_create_failure {};

class glfw {
public:
  using window_type = GLFWwindow;
  using key_callback = GLFWkeyfun;
  using mouse_button_callback = GLFWmousebuttonfun;
  using cursor_callback = GLFWcursorposfun;

  template <typename T>
  static T load_vulkan_function(
      VkInstance instance,
      std::string_view functionName) {
    init();
    return reinterpret_cast<T>(glfwGetInstanceProcAddress(
        instance, functionName.data()));
  }

  static tl::expected<window_type*, window_create_failure>
  create_window(
      int width,
      int height,
      std::string_view windowTitle);

  static void destroy_window(window_type* window);

  static tl::expected<VkSurfaceKHR, VkResult>
  create_surface(VkInstance instance, window_type* window);

  static void set_key_callback(
      window_type* window,
      key_callback callback);

  static void set_mouse_button_callback(
      window_type* window,
      mouse_button_callback callback);

  static void set_cursor_callback(
      window_type* window,
      cursor_callback callback);

  static void set_user_pointer(
      window_type* window,
      void* userPointer) {
    glfwSetWindowUserPointer(window, userPointer);
  }

  template <typename T>
  static T* get_user_pointer(window_type* window) {
    return reinterpret_cast<T*>(
        glfwGetWindowUserPointer(window));
  }

  static window_should_close poll_os(window_type* window);

  static gsl::span<const char*>
  get_required_instance_extensions() {
    init();
    uint32_t count{};
    const char** ptr =
        glfwGetRequiredInstanceExtensions(&count);
    return gsl::span<const char*>{ptr, count};
  }

  static void init();
};
}  // namespace platform