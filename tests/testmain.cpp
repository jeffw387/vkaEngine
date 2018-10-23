#include "Engine.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "VkEnumStrings.hpp"

namespace fs = std::experimental::filesystem;

struct Material {
  glm::vec4 diffuse;
};

struct Light {
  glm::vec4 color;
  glm::vec4 positionViewSpace;
};

struct LightData {
  glm::vec4 ambient;
  uint32_t count;
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

inline void createAssetBuffers(vka::Device* device, vka::gltf::Asset& asset) {
  for (auto& buffer : asset.buffers) {
    buffer.vulkanBuffer = device->createAllocatedBuffer(
        buffer.byteLength,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
  }
}

enum class DescriptorSets {
  Materials,
  DynamicLights,
  AmbientLight,
  Camera,
  Instance
};

struct AppState {
  PolySize defaultWidth = PolySize{900U};
  PolySize defaultHeight = PolySize{900U};
  VkFormat swapFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  vka::OrthoCamera mainCamera;
  std::unique_ptr<vka::Engine> engine;
  vka::Instance* instance;
  vka::Surface* surface;
  vka::Device* device;
  vka::ShaderModule vertexShader;
  vka::ShaderModule fragmentShader;
  vka::Swapchain swapchain;
  vka::RenderPass renderPass;
  std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings;
  std::vector<vka::DescriptorSetLayout> descriptorSetLayouts;
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
    vka::DescriptorPool descriptorPool;
    std::vector<vka::DescriptorSet> descriptorSets;
    vka::vulkan_vector<Light, vka::StorageBufferDescriptor>
        dynamicLightsUniform;
    vka::vulkan_vector<LightData> lightDataUniform;
    vka::vulkan_vector<Camera> cameraUniform;
    vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor> instanceUniform;
    vka::Fence frameAcquired;
    vka::Fence bufferExecuted;
    vka::Semaphore renderComplete;
    uint32_t swapImageIndex;
    vka::UniqueFramebuffer framebuffer;
    vka::CommandPool commandPool;
    vka::CommandBuffer cmd;
    entt::DefaultRegistry ecs;
  };
  std::array<BufferedState, vka::BufferCount> bufState;

  void initCallback(vka::Engine* engine, int32_t initialIndex) {
    MultiLogger::get()->info("Init Callback");
    bufState[initialIndex].instanceUniform.push_back({glm::mat4(1.f)});
    bufState[initialIndex].instanceUniform.flushMemory(device);

    LightData lightData;
    lightData.count = 1;
    lightData.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    bufState[initialIndex].lightDataUniform.push_back(std::move(lightData));
    bufState[initialIndex].lightDataUniform.flushMemory(device);

    bufState[initialIndex].dynamicLightsUniform.push_back({glm::vec4(), {}});
    bufState[initialIndex].dynamicLightsUniform.flushMemory(device);

    materialUniform.push_back({glm::vec4(1.f, 0.f, 0.f, 1.f)});
    materialUniform.flushMemory(device);
  }

  void updateCallback(vka::Engine* engine) {
    auto updateIndex = engine->currentUpdateIndex();
    auto lastUpdateIndex = engine->previousUpdateIndex();
    // MultiLogger::get()->info(
    //     "Update callback: index {}, prior index {}.",
    //     updateIndex,
    //     lastUpdateIndex);
    bufState[updateIndex].instanceUniform.resize(
        bufState[lastUpdateIndex].instanceUniform.size());
    for (auto i = 0U; i < bufState[lastUpdateIndex].instanceUniform.size();
         ++i) {
      bufState[updateIndex].instanceUniform[i] =
          bufState[lastUpdateIndex].instanceUniform[i];
    }
    bufState[lastUpdateIndex].instanceUniform.flushMemory(device);
  }

  void renderCallback(vka::Engine* engine) {
    auto renderIndex = engine->currentRenderIndex();
    // MultiLogger::get()->info("Render callback: index {}.", renderIndex);
    for (auto& set : bufState[renderIndex].descriptorSets) {
      set.validate(*device);
    }
    if (auto index =
            swapchain.acquireImage(bufState[renderIndex].frameAcquired)) {
      bufState[renderIndex].swapImageIndex = index.value();
    } else {
      switch (index.error()) {
        case VK_NOT_READY:
          return;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
          createSwapchain();
          break;
        default:
          MultiLogger::get()->critical(
              "Unrecoverable vulkan error: {}", vka::Results[index.error()]);
          throw std::runtime_error("Unrecoverable vulkan error.");
      }
    }
    bufState[renderIndex].frameAcquired.wait();
    bufState[renderIndex].frameAcquired.reset();
    bufState[renderIndex].bufferExecuted.wait();
    bufState[renderIndex].bufferExecuted.reset();
    bufState[renderIndex].commandPool.reset();
    bufState[renderIndex].cameraUniform[0].projection =
        mainCamera.getProjection();
    bufState[renderIndex].cameraUniform[0].view = mainCamera.getView();
    bufState[renderIndex].cameraUniform.flushMemory(device);

    auto surfaceCapabilities = device->getSurfaceCapabilities();
    bufState[renderIndex].framebuffer = device->createFramebuffer(
        {swapImageViews[bufState[renderIndex].swapImageIndex].get(),
         depthImageView.get()},
        renderPass,
        surfaceCapabilities.currentExtent.width,
        surfaceCapabilities.currentExtent.height);
    bufState[renderIndex].cmd.begin(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        renderPass,
        0,
        bufState[renderIndex].framebuffer.get());
    std::vector<VkClearValue> clearValues = {VkClearValue{{0.f, 0.f, 0.f, 0.f}},
                                             VkClearValue{{0.f, 0U}}};
    bufState[renderIndex].cmd.beginRenderPass(
        renderPass,
        bufState[renderIndex].framebuffer.get(),
        {{0, 0}, surfaceCapabilities.currentExtent},
        clearValues,
        VK_SUBPASS_CONTENTS_INLINE);
    bufState[renderIndex].cmd.setViewport(
        0,
        {{0,
          0,
          static_cast<float>(surfaceCapabilities.currentExtent.width),
          static_cast<float>(surfaceCapabilities.currentExtent.height),
          0,
          1}});
    bufState[renderIndex].cmd.setScissor(
        0,
        {{0,
          0,
          surfaceCapabilities.currentExtent.width,
          surfaceCapabilities.currentExtent.height}});
    bufState[renderIndex].cmd.bindGraphicsPipeline(pipeline);
    bufState[renderIndex].cmd.bindGraphicsDescriptorSets(
        pipelineLayout,
        0,
        {bufState[renderIndex].descriptorSets[0],
         bufState[renderIndex].descriptorSets[1],
         bufState[renderIndex].descriptorSets[2],
         bufState[renderIndex].descriptorSets[3],
         bufState[renderIndex].descriptorSets[4]},
        {0});
    auto vertexBuffers = getVertexBuffers(shapesAsset, 4);
    bufState[renderIndex].cmd.bindIndexBuffer(
        vertexBuffers.indexBuffer.buffer,
        0,
        vertexBuffers.indexBuffer.accessor.componentType);
    bufState[renderIndex].cmd.bindVertexBuffers(
        0,
        {vertexBuffers.positionBuffer.buffer,
         vertexBuffers.normalBuffer.buffer},
        {vertexBuffers.positionBuffer.view.byteOffset,
         vertexBuffers.normalBuffer.view.byteOffset});
    bufState[renderIndex].cmd.drawIndexed(
        vertexBuffers.indexBuffer.accessor.elementCount, 1, 0, 0, 0);
    bufState[renderIndex].cmd.endRenderPass();
    bufState[renderIndex].cmd.end();
    device->queueSubmit(
        {},
        {bufState[renderIndex].cmd},
        {bufState[renderIndex].renderComplete},
        bufState[renderIndex].bufferExecuted);
    auto presentResult = device->presentImage(
        swapchain,
        bufState[renderIndex].swapImageIndex,
        bufState[renderIndex].renderComplete);

    switch (presentResult) {
      case VK_SUCCESS:
        return;
      case VK_ERROR_OUT_OF_DATE_KHR:
      case VK_SUBOPTIMAL_KHR:
        createSwapchain();
        break;
      default:
        MultiLogger::get()->critical(
            "Unrecoverable vulkan error: {}", vka::Results[presentResult]);
        throw std::runtime_error("Unrecoverable vulkan error.");
    }
  }

  void createSwapchain() {
    swapImageViews.clear();
    swapchain.reset();
    swapchain = device->createSwapchain();
    swapImages = std::move(swapchain.getSwapImages());
    for (const auto& swapImage : swapImages) {
      swapImageViews.push_back(device->createImageView2D(
          swapImage, swapFormat, vka::ImageAspect::Color));
    }
    depthImage = device->createAllocatedImage2D(
        device->getSurfaceCapabilities().currentExtent,
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        vka::ImageAspect::Depth);
    depthImageView = device->createImageView2D(
        depthImage.get().image, depthFormat, vka::ImageAspect::Depth);
  }

  AppState() {
    mainCamera.setDimensions(5, 5);

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
    engineCreateInfo.initCallback = [this](auto engine, auto initialIndex) {
      initCallback(engine, initialIndex);
    };
    engineCreateInfo.updateCallback = [this](auto engine) {
      updateCallback(engine);
    };
    engineCreateInfo.renderCallback = [this](auto engine) {
      renderCallback(engine);
    };
    engine = std::make_unique<vka::Engine>(engineCreateInfo);

    MultiLogger::get()->info("creating instance");
    instance = engine->createInstance(instanceCreateInfo);
    MultiLogger::get()->info("creating surface");
    surface = instance->createSurface(surfaceCreateInfo);

    MultiLogger::get()->info("creating device");
    device = instance->createDevice(
        {"VK_KHR_swapchain"}, {}, [&](const vka::PhysicalDeviceData& data) {
          for (const auto& [physicalDevice, props] : data.properties) {
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
              return physicalDevice;
            }
          }
          return data.physicalDevices.at(0);
        });

    createSwapchain();

    MultiLogger::get()->info("loading shapes.gltf");
    shapesAsset = vka::gltf::loadGLTF("content/models/shapes.gltf");
    createAssetBuffers(device, shapesAsset);
    {
      auto assetFence = device->createFence(false);
      auto assetCopyPool = device->createCommandPool();
      auto assetCmd = assetCopyPool.allocateCommandBuffers(1)[0];
      auto byteLength = shapesAsset.buffers[0].byteLength;
      auto assetStageBuffer = device->createAllocatedBuffer(
          byteLength,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VMA_MEMORY_USAGE_CPU_ONLY);
      void* stagePtr{};
      vmaMapMemory(
          device->getAllocator(), assetStageBuffer.get().allocation, &stagePtr);
      std::memcpy(
          stagePtr, shapesAsset.buffers[0].bufferData.get(), byteLength);
      vmaFlushAllocation(
          device->getAllocator(),
          assetStageBuffer.get().allocation,
          0,
          byteLength);

      assetCmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      assetCmd.copyBuffer(
          assetStageBuffer.get().buffer,
          shapesAsset.buffers[0].vulkanBuffer.get().buffer,
          {{0, 0, byteLength}});
      assetCmd.end();
      device->queueSubmit({}, {assetCmd}, {}, assetFence);
      assetFence.wait();
    }

    VkDescriptorSetLayoutBinding materialBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding dynamicLightBinding = {
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding lightDataBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding cameraBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr};
    VkDescriptorSetLayoutBinding instanceBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr};
    MultiLogger::get()->info("creating set layout");
    descriptorSetLayouts.push_back(
        std::move(device->createSetLayout({materialBinding})));
    descriptorSetLayouts.push_back(
        std::move(device->createSetLayout({dynamicLightBinding})));
    descriptorSetLayouts.push_back(
        std::move(device->createSetLayout({lightDataBinding})));
    descriptorSetLayouts.push_back(
        std::move(device->createSetLayout({cameraBinding})));
    descriptorSetLayouts.push_back(
        std::move(device->createSetLayout({instanceBinding})));

    materialUniform = vka::vulkan_vector<Material>(
        device,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU);

    for (auto& state : bufState) {
      MultiLogger::get()->info("creating command pool");
      state.commandPool = device->createCommandPool();
      state.cmd = state.commandPool.allocateCommandBuffers(1)[0];

      state.dynamicLightsUniform =
          vka::vulkan_vector<Light, vka::StorageBufferDescriptor>(
              device,
              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
              VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.dynamicLightsUniform.reserve(1);

      state.lightDataUniform = vka::vulkan_vector<LightData>(
          device,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.lightDataUniform.resize(1);

      state.cameraUniform = vka::vulkan_vector<Camera>(
          device,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.cameraUniform.resize(1);

      state.instanceUniform =
          vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor>(
              device,
              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
              VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.instanceUniform.reserve(1);

      MultiLogger::get()->info("creating descriptor pool");
      state.descriptorPool = device->createDescriptorPool(
          {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
           {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3},
           {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3}},
          5);
      state.descriptorSets.push_back(
          std::move(state.descriptorPool.allocateDescriptorSets(
              {&descriptorSetLayouts[0]})[0]));
      state.descriptorSets.push_back(
          std::move(state.descriptorPool.allocateDescriptorSets(
              {&descriptorSetLayouts[1]})[0]));
      state.descriptorSets.push_back(
          std::move(state.descriptorPool.allocateDescriptorSets(
              {&descriptorSetLayouts[2]})[0]));
      state.descriptorSets.push_back(
          std::move(state.descriptorPool.allocateDescriptorSets(
              {&descriptorSetLayouts[3]})[0]));
      state.descriptorSets.push_back(
          std::move(state.descriptorPool.allocateDescriptorSets(
              {&descriptorSetLayouts[4]})[0]));
      materialUniform.subscribe(
          state.descriptorSets[0].getDescriptor<vka::BufferDescriptor>(
              {VkDescriptorSet{}, 0, 0}));
      state.dynamicLightsUniform.subscribe(
          state.descriptorSets[1].getDescriptor<vka::StorageBufferDescriptor>(
              {VkDescriptorSet{}, 0, 0}));
      state.lightDataUniform.subscribe(
          state.descriptorSets[2].getDescriptor<vka::BufferDescriptor>(
              {VkDescriptorSet{}, 0, 0}));
      state.cameraUniform.subscribe(
          state.descriptorSets[3].getDescriptor<vka::BufferDescriptor>(
              {VkDescriptorSet{}, 0, 0}));
      state.instanceUniform.subscribe(
          state.descriptorSets[4].getDescriptor<vka::DynamicBufferDescriptor>(
              {VkDescriptorSet{}, 0, 0}));

      state.frameAcquired = device->createFence(false);
      state.bufferExecuted = device->createFence(true);
      state.renderComplete = device->createSemaphore();
    }

    MultiLogger::get()->info("creating vertex shader");
    vertexShader =
        device->createShaderModule("content/shaders/shader.vert.spv");
    MultiLogger::get()->info("creating fragment shader");
    fragmentShader =
        device->createShaderModule("content/shaders/shader.frag.spv");
    MultiLogger::get()->info("creating swapchain");

    MultiLogger::get()->info("creating pipeline layout");
    pipelineLayout = device->createPipelineLayout(
        {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}},
        {descriptorSetLayouts[0],
         descriptorSetLayouts[1],
         descriptorSetLayouts[2],
         descriptorSetLayouts[3],
         descriptorSetLayouts[4]});

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
    subpass3D->setDepthRef({depthAttachmentDesc,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

    MultiLogger::get()->info("creating render pass");
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

    MultiLogger::get()->info("creating pipeline cache");
    pipelineCache = device->createPipelineCache();
    MultiLogger::get()->info("creating pipeline");
    pipeline = device->createGraphicsPipeline(pipelineCache, pipeline3DInfo);

    engine->run();
  }
};

int main() {
  AppState appState{};

  return 0;
}