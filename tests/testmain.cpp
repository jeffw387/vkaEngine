#include "Engine.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include <glm/glm.hpp>

#include <memory>

struct Material {
  glm::vec4 diffuse;
};

struct InstanceData {
  glm::mat4 modelMatrix;
  uint32_t modelIndex;
};

int main()
{
  vka::InstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.appName = "testmain";
  instanceCreateInfo.appVersion = {0, 0, 1};
  instanceCreateInfo.instanceExtensions.push_back("VK_KHR_surface");
  instanceCreateInfo.instanceExtensions.push_back("VK_EXT_debug_utils");
  //instanceCreateInfo.layers.push_back("VK_LAYER_LUNARG_standard_validation");
  vka::SurfaceCreateInfo surfaceCreateInfo{};
  surfaceCreateInfo.windowTitle = "testmain window";
  surfaceCreateInfo.width = 900;
  surfaceCreateInfo.height = 900;
  vka::EngineCreateInfo engineCreateInfo{};
  engineCreateInfo.updateCallback = [&](auto engine) {};
  auto engine = std::make_unique<vka::Engine>(engineCreateInfo);
  auto triangleAsset = engine->LoadAsset("Triangle.blend");

  auto instance = engine->createInstance(instanceCreateInfo);
  auto surface = instance->createSurface(surfaceCreateInfo);

  vka::DeviceRequirements deviceRequirements{};
  deviceRequirements.deviceExtensions.push_back("VK_KHR_swapchain");
  auto device = instance->createDevice(deviceRequirements);
  auto vertexShader = device->createShaderModule("vert.spv");
  auto fragmentShader = device->createShaderModule("frag.spv");
  auto swapchain = device->createSwapchain();
  auto commandPool = device->createCommandPool();
  auto cmd = 
    commandPool->allocateCommandBuffers(
      1, 
      VK_COMMAND_BUFFER_LEVEL_PRIMARY)[0];
  
  engine->run();
  return 0; 
}