#include "Engine.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include <glm/glm.hpp>
#include "entt/entt.hpp"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <memory>

struct Material {
  glm::vec4 diffuse;
};

struct Materials {
  std::vector<Material> data;
};

struct Light {
  glm::vec4 color;
  glm::vec4 positionViewSpace;
};

struct LightData {
  std::vector<Light> lights;
  glm::vec4 ambient;
};

struct Camera {
  glm::mat4 view;
  glm::mat4 projection;
};

struct Instance {
  glm::mat4 model;
};

struct InstanceData {
  std::vector<Instance> instances;
  std::vector<uint32_t> materialIndex;
};

struct AllocatedBuffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocInfo;
};

template <typename T, size_t N>
struct BufferedType {
  std::array<T, N> hostData;
  std::array<AllocatedBuffer, N> allocatedBuffers;
};

struct HostRenderState {
  BufferedType<Materials, 1> materialState;
  BufferedType<LightData, 3> lightState;
  BufferedType<Camera, 3> cameraState;
  BufferedType<InstanceData, 3> instanceState;
};

// 0: prior update (data already uploaded to gpu)
// 1: latest update (upload data to gpu, interpolate between prior and latest)
// 2: updating

namespace Components {

struct Mesh {
  size_t meshIndex;
};
struct Material {
  uint32_t materialIndex;
};
struct Physics {
  btRigidBody* rigidBody;
  btCollisionShape* shape;
};
}


int main()
{
  HostRenderState hostRenderState{};
  auto ecsRegistry = entt::DefaultRegistry{};
  
  vka::InstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.appName = "testmain";
  instanceCreateInfo.appVersion = {0, 0, 1};
  instanceCreateInfo.instanceExtensions.push_back("VK_KHR_surface");
  instanceCreateInfo.instanceExtensions.push_back("VK_EXT_debug_utils");
  instanceCreateInfo.layers.push_back("VK_LAYER_LUNARG_standard_validation");
  vka::SurfaceCreateInfo surfaceCreateInfo{};
  surfaceCreateInfo.windowTitle = "testmain window";
  surfaceCreateInfo.width = 900;
  surfaceCreateInfo.height = 900;
  vka::EngineCreateInfo engineCreateInfo{};
  engineCreateInfo.updateCallback = [&](auto engine) {};
  auto engine = std::make_unique<vka::Engine>(engineCreateInfo);
  auto triangleAsset = engine->LoadAsset("content/models/triangle.blend");

  auto instance = engine->createInstance(instanceCreateInfo);
  auto surface = instance->createSurface(surfaceCreateInfo);

  vka::DeviceRequirements deviceRequirements{};
  deviceRequirements.deviceExtensions.push_back("VK_KHR_swapchain");
  auto device = instance->createDevice(deviceRequirements);
  auto vertexShader = device->createShaderModule("content/shaders/vert.spv");
  auto fragmentShader = device->createShaderModule("content/shaders/frag.spv");
  auto swapchain = device->createSwapchain();
  auto commandPool = device->createCommandPool();
  auto cmd = 
    commandPool->allocateCommandBuffers(
      1, 
      VK_COMMAND_BUFFER_LEVEL_PRIMARY)[0];
  
  engine->run();
  return 0; 
}