#pragma once

#include <vulkan/vulkan.h>
#include "GLFW.hpp"
#include <memory>
#include <variant>
#include <vector>
#include <string_view>
#include <expected.hpp>

namespace vka {
using WindowType = platform::GLFW::WindowType;
class surface {
public:
  explicit surface(VkInstance instance, WindowType* window, VkSurfaceKHR surface_handle) 
    : m_instance(instance), m_window(window), m_surface(surface_handle) {
  }

  ~surface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    platform::GLFW::destroyWindow(m_window);
  }
  
  operator VkSurfaceKHR() { return m_surface; }

  operator WindowType*() { return m_window; }

private:
  VkInstance m_instance;
  WindowType* m_window;
  VkSurfaceKHR m_surface;
};

using surface_error = std::variant<VkResult, platform::WindowCreateFailure>;

class surface_builder {
public:
  tl::expected<std::unique_ptr<surface>, surface_error> build(VkInstance instance) {
    if (auto window_result = platform::GLFW::createWindow(m_width, m_height, m_window_title)) {
      if (auto surface_result = platform::GLFW::createSurface(instance, window_result.value())) {
        return std::make_unique<surface>(instance, window_result.value(), surface_result.value());
      } 
      else {
        return tl::make_unexpected(surface_result.error());
      } 
    }
    else {
      return tl::make_unexpected(window_result.error());
    }
  }

  surface_builder& width(int value) {
    m_width = value;
    return *this;
  }

  surface_builder& height(int value) {
    m_height = value;
    return *this;
  }

  surface_builder& title(std::string_view window_title) {
    m_window_title = window_title;
    return *this;
  }

private:
  int m_width = {};
  int m_height = {};
  std::string_view m_window_title = {};
};
}  // namespace vka