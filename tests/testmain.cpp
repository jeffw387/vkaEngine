#include "Engine.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include <glm/glm.hpp>
#include "entt/entt.hpp"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <memory>
#include "Camera.hpp"

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

template <typename T, size_t N>
struct BufferedType {
  std::array<T, N> hostData;
  std::array<vka::AllocatedBuffer, N> allocatedBuffers;
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
}  // namespace Components

struct FragmentSpecData {
  uint32_t materialCount;
  uint32_t lightCount;
};

struct FragmentPushConstants {
  uint32_t materialIndex;
};

int main() {
  const uint32_t defaultWidth = 900U;
  const uint32_t defaultHeight = 900U;
  HostRenderState hostRenderState{};
  Materials materials;
  materials.data.push_back({glm::vec4(1.f, 0.f, 0.f, 1.f)});
  hostRenderState.materialState.hostData[0] = materials;
  auto mainCamera = vka::OrthoCamera{};
  mainCamera.setDimensions(900.f, 900.f);
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
  engineCreateInfo.updateCallback = [&](vka::Engine* engine) {
    auto updateIndex = engine->currentUpdateIndex();

    // hostRenderState.cameraState.hostData[updateIndex].view = ;
  };
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
  auto cmd = commandPool->allocateCommandBuffers(
      1, VK_COMMAND_BUFFER_LEVEL_PRIMARY)[0];

  std::vector<VkDescriptorSetLayoutBinding> set3Dbindings;
  VkDescriptorSetLayoutBinding materialBinding = {
      0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      1,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      nullptr};
  set3Dbindings.push_back(materialBinding);
  VkDescriptorSetLayoutBinding lightBinding = {
      1,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      1,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      nullptr};
  set3Dbindings.push_back(lightBinding);
  VkDescriptorSetLayoutBinding cameraBinding = {
      2,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      1,
      VK_SHADER_STAGE_VERTEX_BIT,
      nullptr};
  set3Dbindings.push_back(cameraBinding);
  VkDescriptorSetLayoutBinding instanceBinding = {
      3,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
      1,
      VK_SHADER_STAGE_VERTEX_BIT,
      nullptr};
  set3Dbindings.push_back(instanceBinding);
  auto setLayout3D = device->createSetLayout(set3Dbindings);
  std::vector<VkDescriptorPoolSize> poolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3}};
  auto setPool = device->createDescriptorPool(poolSizes, 3);
  std::vector<VkPushConstantRange> pushRanges = {
      {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}};
  std::vector<VkDescriptorSetLayout> setLayouts = {*setLayout3D};
  auto pipeline3Dlayout = device->createPipelineLayout(pushRanges, setLayouts);

  vka::RenderPassCreateInfo renderPassCreateInfo;
  auto renderPass3D = device->createRenderPass(renderPassCreateInfo);
  auto pipeline3DInfo =
      vka::GraphicsPipelineCreateInfo(*pipeline3Dlayout, *renderPass3D, 0);
  pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
  std::vector<VkSpecializationMapEntry> vertexMapEntries;
  pipeline3DInfo.addShaderStage(
      VK_SHADER_STAGE_VERTEX_BIT,
      vertexMapEntries,
      0,
      nullptr,
      *vertexShader,
      "main");
  std::vector<VkSpecializationMapEntry> fragmentMapEntries = {{0, 0, 4},
                                                              {1, 4, 4}};
  FragmentSpecData fragmentSpecData{1, 1};
  pipeline3DInfo.addShaderStage(
      VK_SHADER_STAGE_FRAGMENT_BIT,
      fragmentMapEntries,
      sizeof(FragmentSpecData),
      &fragmentSpecData,
      *fragmentShader,
      "main");
  auto pipeline3D = device->createGraphicsPipeline(pipeline3DInfo);

  // device->createAllocatedBuffer()

  engine->run();
  return 0;
}