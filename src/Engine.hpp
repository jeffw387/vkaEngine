#pragma once
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <taskflow/taskflow.hpp>

#include "Clock.hpp"
#include "Logger.hpp"
#include "Input.hpp"

namespace vka {
static constexpr auto BufferCount = 3U;

class Engine;
class Surface;
class Instance;
struct InstanceCreateInfo;



class Engine {
public:
  std::unique_ptr<Instance> createInstance(InstanceCreateInfo);

  Input::Manager inputManager;
  double mouseX;
  double mouseY;

private:
  std::unique_ptr<GLFW> glfwInstance;
};
}  // namespace vka