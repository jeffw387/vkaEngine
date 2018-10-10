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
#include <cstring>
#include <vector>
#include "Camera.hpp"
#include "Asset.hpp"
#include "vulkan_vector.hpp"

struct Material {
  glm::vec4 diffuse;
};

// template <typename ContentType, size_t Count>
// class MutableBuffer {
//   VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//   std::array<std::vector<ContentType>, Count> content;
//   std::array<bool, Count> dataValid;
//   std::array<vka::UniqueAllocatedBuffer, Count> contentBuffers;
//   std::array<size_t, Count> bufferCapacities;
//   std::array<void*, Count> mapPtrs;

//   template <size_t Index>
//   size_t contentSize() const {
//     return sizeof(ContentType) * content[Index].size();
//   }

// public:
//   MutableBuffer() = default;
//   MutableBuffer(VkBufferUsageFlags bufferUsage) : bufferUsage(bufferUsage) {}

//   template <size_t Index>
//   size_t size() const {
//     return content[Index].size();
//   }

//   template <size_t Index>
//   void push_back(ContentType value) {
//     content[Index].push_back(value);
//   }

//   template <size_t Index>
//   void push_back(ContentType&& value) {
//     content[Index].push_back(std::move(value));
//     dataValid[Index] = false;
//   }

//   template <size_t Index>
//   ContentType& operator[](size_t contentIndex) {
//     return content[Index][contentIndex];
//   }

//   template <size_t Index>
//   const ContentType& operator[](size_t contentIndex) const {
//     return content[Index][contentIndex];
//   }

//   template <size_t Index>
//   void validateBuffer(vka::Device* device) {
//     // pseudo:
//     // if (resize needed) {resize}
//     if (size<Index>() > bufferCapacities[Index]) {
//       createBuffer<Index>(device):
//     }
//     // if (data copy needed) {copy data}
//     if (!dataValid[Index]) {
//       validateData<Index>();
//     }
//   }

// private:
//   template <size_t Index>
//   void createBuffer(vka::Device* device) {
//     contentBuffers[Index] = device->createAllocatedBuffer(
//         contentSize(),
//         bufferUsage,
//         VMA_MEMORY_USAGE_CPU_TO_GPU);
//     vmaMapMemory(
//       device->getAllocator(),
//       (*contentBuffers[Index]).allocation,
//       &mapPtrs[Index]);
//   }

//   template <size_t Index>
//   void validateData() {
//     std::memcpy(mapPtrs[Index], content[Index].data(), sizeof(ContentType));
//     dataValid[Index] = true;
//   }

//   template <size_t Index>
//   void validateBufferCapacity() {
//     if (contentCount() <)
//   }
//   template <size_t Index>
//   void validateBufferData() {}

// };

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

// template <typename T, size_t N>
// struct BufferedType {
//   std::array<T, N> hostData;
//   std::array<vka::UniqueAllocatedBuffer, N> allocatedBuffers;
//   std::array<VkDescriptorBufferInfo, N> bufferInfos;

//   struct Count {
//     constexpr size_t operator()() {
//       return N;
//     }
//   };
// };

// struct HostRenderState {
//   BufferedType<Materials, 1> materialState;
//   BufferedType<LightData, 3> lightState;
//   BufferedType<Camera, 3> cameraState;
//   BufferedType<InstanceData, 3> instanceState;
//   std::array<vka::DescriptorPool, 3> descriptorPools;
//   std::array<vka::DescriptorSet, 3> descriptorSets;
// };

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

struct PolySize {
  const size_t size;
  template <typename T>
  operator T() {
    return static_cast<T>(size);
  }
};

int main() {
  PolySize defaultWidth{900U};
  PolySize defaultHeight{900U};
  // MaterialData materialData;
  // materialData.addMaterial({glm::vec4(1.f, 0.f, 0.f, 1.f)});

  LightData lightData{};
  lightData.lights.resize(1);
  lightData.ambient = {1.f, 1.f, 1.f, 0.3f};

  // HostRenderState hostRenderState{};
  // hostRenderState.materialState.hostData[0] = materials;
  auto mainCamera = vka::OrthoCamera{};
  mainCamera.setDimensions(defaultWidth, defaultHeight);
  auto ecsRegistry = entt::DefaultRegistry{};

  vka::InstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.appName = "testmain";
  instanceCreateInfo.appVersion = {0, 0, 1};
  instanceCreateInfo.instanceExtensions.push_back("VK_KHR_surface");
  instanceCreateInfo.instanceExtensions.push_back("VK_EXT_debug_utils");
  instanceCreateInfo.layers.push_back("VK_LAYER_LUNARG_standard_validation");
  vka::SurfaceCreateInfo surfaceCreateInfo{};
  surfaceCreateInfo.windowTitle = "testmain window";
  surfaceCreateInfo.width = defaultWidth;
  surfaceCreateInfo.height = 900;
  vka::EngineCreateInfo engineCreateInfo{};
  engineCreateInfo.updateCallback = [&](vka::Engine* engine) {
    auto updateIndex = engine->currentUpdateIndex();
  };
  engineCreateInfo.renderCallback = [&](vka::Engine* engine) {
    auto renderIndex = engine->currentRenderIndex();
    // hostRenderState.cameraState.hostData[renderIndex].view =
    //     mainCamera.getView();
    // hostRenderState.cameraState.hostData[renderIndex].projection =
    //     mainCamera.getProjection();
    // hostRenderState.cameraState.allocatedBuffers[renderIndex]
  };
  auto engine = std::make_unique<vka::Engine>(engineCreateInfo);
  auto multilogger = spdlog::get(vka::LoggerName);
  multilogger->info("loading shapes.gltf");
  auto shapesAsset = vka::gltf::loadGLTF("content/models/shapes.gltf");

  multilogger->info("creating instance");
  auto instance = engine->createInstance(instanceCreateInfo);
  multilogger->info("creating surface");
  auto surface = instance->createSurface(surfaceCreateInfo);

  vka::DeviceRequirements deviceRequirements{};
  deviceRequirements.deviceExtensions.push_back("VK_KHR_swapchain");
  multilogger->info("creating device");
  auto device = instance->createDevice(deviceRequirements);

  auto materials = vka::vulkan_vector<Material>(
      device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  materials.push_back({glm::vec4(1.f, 0.f, 0.f, 1.f)});

  multilogger->info("creating vertex shader");
  auto vertexShader = device->createShaderModule("content/shaders/shader.vert");
  multilogger->info("creating fragment shader");
  auto fragmentShader =
      device->createShaderModule("content/shaders/shader.frag");
  multilogger->info("creating swapchain");
  auto swapchain = device->createSwapchain();
  multilogger->info("creating command pool");
  auto commandPool = device->createCommandPool();
  multilogger->info("allocating command buffer");
  auto cmd =
      commandPool.allocateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY)[0];

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
  multilogger->info("creating set layout");
  auto setLayout3D = device->createSetLayout(set3Dbindings);
  std::vector<VkDescriptorPoolSize> poolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3}};
  multilogger->info("creating descriptor pool");
  auto setPool = device->createDescriptorPool(poolSizes, 3);
  std::vector<VkPushConstantRange> pushRanges = {
      {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}};
  std::vector<VkDescriptorSetLayout> setLayouts = {setLayout3D};
  multilogger->info("creating pipeline layout");
  auto pipeline3Dlayout = device->createPipelineLayout(pushRanges, setLayouts);

  vka::RenderPassCreateInfo renderPassCreateInfo;
  auto colorAttachmentDesc = renderPassCreateInfo.addAttachmentDescription(
      0,
      VK_FORMAT_B8G8R8A8_UNORM,
      VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  auto depthAttachmentDesc = renderPassCreateInfo.addAttachmentDescription(
      0,
      VK_FORMAT_D32_SFLOAT,
      VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  auto subpass3D = renderPassCreateInfo.addGraphicsSubpass();
  subpass3D->addColorRef(
      {colorAttachmentDesc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
  subpass3D->setDepthRef(
      {depthAttachmentDesc,
       VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR});

  multilogger->info("creating render pass");
  auto renderPass3D = device->createRenderPass(renderPassCreateInfo);
  auto pipeline3DInfo =
      vka::GraphicsPipelineCreateInfo(pipeline3Dlayout, renderPass3D, 0);
  pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
  std::vector<VkSpecializationMapEntry> vertexMapEntries;
  pipeline3DInfo.addShaderStage(
      VK_SHADER_STAGE_VERTEX_BIT,
      vertexMapEntries,
      0,
      nullptr,
      vertexShader,
      "main");
  std::vector<VkSpecializationMapEntry> fragmentMapEntries = {{0, 0, 4},
                                                              {1, 4, 4}};
  FragmentSpecData fragmentSpecData{1, 1};
  pipeline3DInfo.addShaderStage(
      VK_SHADER_STAGE_FRAGMENT_BIT,
      fragmentMapEntries,
      sizeof(FragmentSpecData),
      &fragmentSpecData,
      fragmentShader,
      "main");
  pipeline3DInfo.addVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
  pipeline3DInfo.addVertexAttribute(1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0);
  pipeline3DInfo.addVertexBinding(
      0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX);
  pipeline3DInfo.addVertexBinding(
      1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX);
  pipeline3DInfo.addViewportScissor(VkViewport{}, VkRect2D{});
  pipeline3DInfo.addColorBlendAttachment(
      false,
      VkBlendFactor(0),
      VkBlendFactor(0),
      VkBlendOp(0),
      VkBlendFactor(0),
      VkBlendFactor(0),
      VkBlendOp(0),
      0);

  multilogger->info("creating pipeline cache");
  auto pipelineCache = device->createPipelineCache();
  multilogger->info("creating pipeline");
  auto pipeline3D =
      device->createGraphicsPipeline(pipelineCache, pipeline3DInfo);

  // for (auto& allocBuffer : hostRenderState.materialState.allocatedBuffers) {
  //   allocBuffer = device->createAllocatedBuffer(
  //       sizeof(Material) * materials.data.size(),
  //       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //       0,
  //       VMA_MEMORY_USAGE_GPU_ONLY);
  // }

  // for (auto& allocBuffer : hostRenderState.lightState.allocatedBuffers) {
  //   allocBuffer = device->createAllocatedBuffer(
  //       sizeof(Light) * lightData.lights.size(),
  //       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //       VMA_ALLOCATION_CREATE_MAPPED_BIT,
  //       VMA_MEMORY_USAGE_CPU_TO_GPU);
  // }

  // for (auto& allocBuffer : hostRenderState.cameraState.allocatedBuffers) {
  //   allocBuffer = device->createAllocatedBuffer(
  //       sizeof(Camera),
  //       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //       VMA_ALLOCATION_CREATE_MAPPED_BIT,
  //       VMA_MEMORY_USAGE_CPU_TO_GPU);
  // }

  // for (auto& allocBuffer : hostRenderState.instanceState.allocatedBuffers) {
  //   allocBuffer = device->createAllocatedBuffer(
  //       sizeof(Instance),
  //       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //       VMA_ALLOCATION_CREATE_MAPPED_BIT,
  //       VMA_MEMORY_USAGE_CPU_TO_GPU);
  // }

  engine->run();
  return 0;
}