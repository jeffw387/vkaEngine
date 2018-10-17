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
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

struct Material {
  glm::vec4 diffuse;
};

struct Light {
  glm::vec4 color;
  glm::vec4 positionViewSpace;
};

struct AmbientLight {
  glm::vec4 color;
};

struct Camera {
  glm::mat4 view;
  glm::mat4 projection;
};

struct Instance {
  glm::mat4 model;
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

struct PolySize {
  const size_t size;
  template <typename T>
  operator T() {
    return static_cast<T>(size);
  }
};

struct AppState {
  PolySize defaultWidth = PolySize{900U};
  PolySize defaultHeight = PolySize{900U};
  vka::OrthoCamera mainCamera;
  std::unique_ptr<vka::Engine> engine;
  std::shared_ptr<spdlog::logger> multilogger;
  vka::Instance* instance;
  vka::Surface* surface;
  vka::Device* device;
  vka::ShaderModule vertexShader;
  vka::ShaderModule fragmentShader;
  vka::Swapchain swapchain;
  vka::RenderPass renderPass;
  std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings;
  vka::DescriptorSetLayout descriptorSetLayout;
  vka::PipelineLayout pipelineLayout;
  vka::PipelineCache pipelineCache;
  vka::GraphicsPipeline pipeline;

  vka::gltf::Asset shapesAsset;
  vka::vulkan_vector<Material> materialUniform;
  vka::UniqueAllocatedBuffer indexBuffer;
  vka::UniqueAllocatedBuffer positionBuffer;
  vka::UniqueAllocatedBuffer normalBuffer;
  std::vector<VkImage> swapImages;
  std::vector<vka::UniqueImageView> swapImageViews;
  vka::UniqueAllocatedImage depthImage;
  vka::UniqueImageView depthImageView;

  struct BufferedState {
    vka::CommandPool commandPool;
    vka::DescriptorPool descriptorPool;
    // if light count changes, pipeline must be recompiled (ideally only change
    // on area change)
    vka::vulkan_vector<Light> dynamicLightsUniform;
    vka::vulkan_vector<Light> ambientLightUniform;
    vka::vulkan_vector<Camera> cameraUniform;
    vka::vulkan_vector<Instance> instanceUniform;
    VkSemaphore frameAcquired;
    VkSemaphore renderComplete;
    uint32_t swapImageIndex;
    vka::UniqueFramebuffer framebuffer;
    entt::DefaultRegistry ecs;
  };
  std::array<BufferedState, vka::BufferCount> bufState;

  AppState() {
    mainCamera.setDimensions(defaultWidth, defaultHeight);

    vka::InstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.appName = "testmain";
    instanceCreateInfo.appVersion = {0, 0, 1};
    instanceCreateInfo.instanceExtensions.push_back("VK_KHR_surface");
    instanceCreateInfo.instanceExtensions.push_back("VK_EXT_debug_utils");
    instanceCreateInfo.layers.push_back("VK_LAYER_LUNARG_standard_validation");
    // instanceCreateInfo.layers.push_back("VK_LAYER_LUNARG_api_dump");

    vka::SurfaceCreateInfo surfaceCreateInfo{};
    surfaceCreateInfo.windowTitle = "testmain window";
    surfaceCreateInfo.width = defaultWidth;
    surfaceCreateInfo.height = defaultHeight;

    vka::EngineCreateInfo engineCreateInfo{};
    engineCreateInfo.updateCallback = [&](vka::Engine* engine) {
      auto updateIndex = engine->currentUpdateIndex();
    };
    engineCreateInfo.renderCallback = [&](vka::Engine* engine) {
      auto renderIndex = engine->currentRenderIndex();
      bufState[renderIndex].cameraUniform[0].projection =
          mainCamera.getProjection();
      bufState[renderIndex].cameraUniform[0].view = mainCamera.getView();
      bufState[renderIndex].cameraUniform.flushMemory(device);
    };
    engine = std::make_unique<vka::Engine>(engineCreateInfo);
    multilogger = spdlog::get(vka::LoggerName);
    multilogger->info("loading shapes.gltf");
    shapesAsset = vka::gltf::loadGLTF("content/models/shapes.gltf");

    multilogger->info("creating instance");
    instance = engine->createInstance(instanceCreateInfo);
    multilogger->info("creating surface");
    surface = instance->createSurface(surfaceCreateInfo);

    vka::DeviceRequirements deviceRequirements{};
    deviceRequirements.deviceExtensions.push_back("VK_KHR_swapchain");
    multilogger->info("creating device");
    device = instance->createDevice(deviceRequirements);

    materialUniform = vka::vulkan_vector<Material>(
        device,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU);
    materialUniform.push_back({glm::vec4(1.f, 0.f, 0.f, 1.f)});

    auto currentPath = fs::current_path();
    multilogger->info("creating vertex shader");
    vertexShader =
        device->createShaderModule("content/shaders/shader.vert.spv");
    multilogger->info("creating fragment shader");
    fragmentShader =
        device->createShaderModule("content/shaders/shader.frag.spv");
    multilogger->info("creating swapchain");
    swapchain = device->createSwapchain();
    for (auto& state : bufState) {
      multilogger->info("creating command pool");
      state.commandPool = device->createCommandPool();
      multilogger->info("creating descriptor pool");
      state.descriptorPool = device->createDescriptorPool(
          {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
           {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3}},
          1);
    }

    VkDescriptorSetLayoutBinding materialBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding dynamicLightBinding = {
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding ambientLightBinding = {
        2,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding cameraBinding = {
        3,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding instanceBinding = {
        4,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr};
    multilogger->info("creating set layout");
    descriptorSetLayout = device->createSetLayout({materialBinding,
                                                   dynamicLightBinding,
                                                   ambientLightBinding,
                                                   cameraBinding,
                                                   instanceBinding});

    multilogger->info("creating pipeline layout");
    pipelineLayout = device->createPipelineLayout(
        {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}}, {descriptorSetLayout});

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
    renderPass = device->createRenderPass(renderPassCreateInfo);
    auto pipeline3DInfo =
        vka::GraphicsPipelineCreateInfo(pipelineLayout, renderPass, 0);
    pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipeline3DInfo.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT, {}, 0, nullptr, vertexShader, "main");
    FragmentSpecData fragmentSpecData{1, 1};
    pipeline3DInfo.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT,
        {{0, 0, 4}, {1, 4, 4}},
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
    pipelineCache = device->createPipelineCache();
    multilogger->info("creating pipeline");
    pipeline = device->createGraphicsPipeline(pipelineCache, pipeline3DInfo);

    engine->run();
  }
};

int main() {
  AppState appState{};

  return 0;
}