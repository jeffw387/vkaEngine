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
  vka::CommandPool transferCommandPool;
  vka::CommandBuffer transferCmd;
  vka::Fence transferFence;
  vka::Swapchain swapchain;
  vka::ShaderModule vertexShader;
  vka::ShaderModule fragmentShader;
  vka::RenderPass renderPass;
  std::vector<vka::DescriptorSetLayout> descriptorSetLayouts;
  vka::PipelineLayout pipelineLayout;
  vka::PipelineCache pipelineCache;
  vka::GraphicsPipeline pipeline;

  vka::asset::Collection shapesAsset;
  vka::asset::Collection terrainAsset;
  std::vector<VkImage> swapImages;
  std::vector<vka::UniqueImageView> swapImageViews;
  vka::UniqueAllocatedImage depthImage;
  vka::UniqueImageView depthImageView;

  struct BufferedState {
    vka::DescriptorPool descriptorPool;
    std::vector<vka::DescriptorSet> descriptorSets;
    vka::vulkan_vector<Material, vka::StorageBufferDescriptor> materialUniform;
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
    auto& initial = bufState[initialIndex];

    initial.materialUniform.push_back({glm::vec4(0.8f, 1.f, 0.8f, 1.f)});
    initial.materialUniform.flushMemory(device);

    initial.dynamicLightsUniform.push_back(
        {{0.8f, 0.8f, 0.8f, 25.f}, {0.f, 0.f, 2.f, 0.f}});
    initial.dynamicLightsUniform.flushMemory(device);

    LightData lightData;
    lightData.count = 1;
    lightData.ambient = glm::vec4(0.f, 0.f, 1.f, 10.0f);
    initial.lightDataUniform.push_back(std::move(lightData));
    initial.lightDataUniform.flushMemory(device);

    Camera camData{};
    camData.projection = mainCamera.getProjection();
    camData.view = mainCamera.getView();
    initial.cameraUniform.push_back(std::move(camData));
    initial.cameraUniform.flushMemory(device);

    initial.instanceUniform.push_back(
        {glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f))});
    initial.instanceUniform.flushMemory(device);
  }

  void updateCallback(vka::Engine* engine) {
    auto updateIndex = engine->currentUpdateIndex();
    auto lastUpdateIndex = engine->previousUpdateIndex();
    auto& last = bufState[lastUpdateIndex];
    auto& current = bufState[updateIndex];

    auto matSize = last.materialUniform.size();
    current.materialUniform.resize(matSize);
    for (auto i = 0U; i < matSize; ++i) {
      current.materialUniform[i] = last.materialUniform[i];
    }
    current.materialUniform.flushMemory(device);

    auto dynamicLightSize = last.dynamicLightsUniform.size();
    current.dynamicLightsUniform.resize(dynamicLightSize);
    for (auto i = 0U; i < dynamicLightSize; ++i) {
      current.dynamicLightsUniform[i] = last.dynamicLightsUniform[i];
    }
    current.dynamicLightsUniform.flushMemory(device);

    current.lightDataUniform.resize(1);
    current.lightDataUniform[0] = last.lightDataUniform[0];
    current.lightDataUniform.flushMemory(device);

    current.cameraUniform.resize(1);
    current.cameraUniform[0].projection = mainCamera.getProjection();
    current.cameraUniform[0].view = mainCamera.getView();
    current.cameraUniform.flushMemory(device);

    auto instanceSize = last.instanceUniform.size();
    current.instanceUniform.resize(instanceSize);
    for (auto i = 0U; i < instanceSize; ++i) {
      current.instanceUniform[i] = last.instanceUniform[i];
    }
    current.instanceUniform.flushMemory(device);
  }

  void renderCallback(vka::Engine* engine) {
    auto renderIndex = engine->currentRenderIndex();
    auto& render = bufState[renderIndex];

    if (auto index = swapchain.acquireImage(render.frameAcquired)) {
      render.swapImageIndex = index.value();
    } else {
      switch (index.error()) {
        case VK_NOT_READY:
          return;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
          device->waitIdle();
          createSwapchain();
          return;
        default:
          MultiLogger::get()->critical(
              "Unrecoverable vulkan error: {}", vka::Results[index.error()]);
          throw std::runtime_error("Unrecoverable vulkan error.");
      }
    }
    render.frameAcquired.wait();
    render.frameAcquired.reset();
    render.bufferExecuted.wait();
    render.bufferExecuted.reset();
    for (auto& set : render.descriptorSets) {
      set.validate(*device);
    }
    render.commandPool.reset();

    auto swapExtent = swapchain.getSwapExtent();
    if (swapExtent.width == 0 || swapExtent.height == 0) {
      return;
    }
    render.framebuffer = device->createFramebuffer(
        {swapImageViews[render.swapImageIndex].get(), depthImageView.get()},
        renderPass,
        swapExtent.width,
        swapExtent.height);
    render.cmd.begin(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        renderPass,
        0,
        render.framebuffer.get());
    std::vector<VkClearValue> clearValues = {VkClearValue{{0.f, 0.f, 0.f, 1.f}},
                                             VkClearValue{{1.f, 0U}}};
    render.cmd.beginRenderPass(
        renderPass,
        render.framebuffer.get(),
        {{0, 0}, swapExtent},
        clearValues,
        VK_SUBPASS_CONTENTS_INLINE);
    render.cmd.setViewport(
        0,
        {{0,
          0,
          static_cast<float>(swapExtent.width),
          static_cast<float>(swapExtent.height),
          0,
          1}});
    render.cmd.setScissor(0, {{0, 0, swapExtent.width, swapExtent.height}});
    render.cmd.bindGraphicsPipeline(pipeline);
    render.cmd.bindGraphicsDescriptorSets(
        pipelineLayout,
        0,
        {render.descriptorSets[0],
         render.descriptorSets[1],
         render.descriptorSets[2],
         render.descriptorSets[3],
         render.descriptorSets[4]},
        {0});
    auto shapesBuffer = shapesAsset.buffer.get().buffer;
    auto someModel = shapesAsset.models[0];
    render.cmd.bindIndexBuffer(
        shapesBuffer, someModel.indexByteOffset, VK_INDEX_TYPE_UINT16);
    render.cmd.bindVertexBuffers(
        0,
        {shapesBuffer, shapesBuffer},
        {someModel.positionByteOffset, someModel.normalByteOffset});
    // render.cmd.drawIndexed(someModel.indexCount, 1, 0, 0, 0);
    uint32_t matIndex{};
    render.cmd.pushConstants(
        pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &matIndex);
    auto terrainBuffer = terrainAsset.buffer.get().buffer;
    render.cmd.bindIndexBuffer(
        terrainBuffer,
        terrainAsset.models[0].indexByteOffset,
        VK_INDEX_TYPE_UINT16);
    render.cmd.bindVertexBuffers(
        0,
        {terrainBuffer, terrainBuffer},
        {terrainAsset.models[0].positionByteOffset,
         terrainAsset.models[0].normalByteOffset});
    render.cmd.drawIndexed(terrainAsset.models[0].indexCount, 1, 0, 0, 0);
    render.cmd.endRenderPass();
    render.cmd.end();
    device->queueSubmit(
        {}, {render.cmd}, {render.renderComplete}, render.bufferExecuted);
    auto presentResult = device->presentImage(
        swapchain, render.swapImageIndex, render.renderComplete);

    switch (presentResult) {
      case VK_SUCCESS:
        return;
      case VK_ERROR_OUT_OF_DATE_KHR:
      case VK_SUBOPTIMAL_KHR:
        device->waitIdle();
        createSwapchain();
        return;
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
        swapchain.getSwapExtent(),
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        vka::ImageAspect::Depth);
    depthImageView = device->createImageView2D(
        depthImage.get().image, depthFormat, vka::ImageAspect::Depth);
  }

  void recordImageUpload(unsigned char* data, size_t bufferSize, VkImage image, VkExtent2D imageExtent) {
    auto staging = device->createAllocatedBuffer(
        bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* stagePtr{};
    vmaMapMemory(device->getAllocator(), staging.get().allocation, &stagePtr);
    std::memcpy(stagePtr, data, bufferSize);
    vmaFlushAllocation(
        device->getAllocator(), staging.get().allocation, 0, bufferSize);

    VkImageSubresourceLayers imageSubresource = {};
    imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = 1;
    imageSubresource.mipLevel = 0;
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;
    
    VkImageMemoryBarrier preCopyBarrier{};
    preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    preCopyBarrier.image = image;
    preCopyBarrier.subresourceRange = subresourceRange;
    preCopyBarrier.srcAccessMask = 0;
    preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transferCmd.pipelineBarrier(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        {},
        {},
        {preCopyBarrier});

    VkBufferImageCopy copy{};
    copy.imageExtent = {imageExtent.width, imageExtent.height, 1};
    copy.imageSubresource = imageSubresource;
    transferCmd.copyBufferToImage(
        staging.get().buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        {copy});

    VkImageMemoryBarrier postCopyBarrier{};
    postCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postCopyBarrier.image = image;
    postCopyBarrier.subresourceRange = subresourceRange;
    postCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    postCopyBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    postCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    postCopyBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    transferCmd.pipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        {},
        {},
        {postCopyBarrier});
  }

  AppState() {
    mainCamera.setDimensions(2, 2);
    mainCamera.setPosition({0.f, 0.f, 0.f});
    mainCamera.setNearFar(-10, 10);

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

    shapesAsset =
        vka::asset::loadCollection(device, "content/models/shapes.gltf");
    terrainAsset =
        vka::asset::loadCollection(device, "content/models/terrain.gltf");

    VkDescriptorSetLayoutBinding materialBinding = {
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
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

    for (auto& state : bufState) {
      MultiLogger::get()->info("creating command pool");
      state.commandPool = device->createCommandPool();
      state.cmd = state.commandPool.allocateCommandBuffers(1)[0];

      state.materialUniform =
          vka::vulkan_vector<Material, vka::StorageBufferDescriptor>(
              device,
              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
              VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.materialUniform.reserve(1);

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
      state.lightDataUniform.reserve(1);

      state.cameraUniform = vka::vulkan_vector<Camera>(
          device,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU);
      state.cameraUniform.reserve(1);

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
      state.materialUniform.subscribe(
          state.descriptorSets[0].getDescriptor<vka::StorageBufferDescriptor>(
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
        swapFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    auto depthAttachmentDesc = renderPassCreateInfo.addAttachmentDescription(
        0,
        depthFormat,
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
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    pipeline3DInfo.setDepthTestEnable(true);
    pipeline3DInfo.setDepthWriteEnable(true);
    pipeline3DInfo.setDepthCompareOp(VK_COMPARE_OP_LESS);
    pipeline3DInfo.setDepthBounds(0.f, 1.f);
    pipeline3DInfo.setCullMode(VK_CULL_MODE_BACK_BIT);
    pipeline3DInfo.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);

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