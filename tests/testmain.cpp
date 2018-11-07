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
#include <memory>
#include <cstring>
#include <mutex>
#include <vector>
#include "Camera.hpp"
#include "Asset.hpp"
#include "vulkan_vector.hpp"
#include <experimental/filesystem>
#include "VkEnumStrings.hpp"
#include "Text.hpp"
#include <range/v3/all.hpp>

namespace fs = std::experimental::filesystem;

using TextIndex = uint16_t;
struct TextVertex {
  glm::vec2 pos;
  glm::vec2 uv;
};

struct CharacterData {
  int32_t xOffset;
  FT_ULong character;
};

struct TextObject {
  glm::vec2 screenPosition;
  std::vector<CharacterData> characters;
  TextObject(
      std::map<FT_ULong, std::unique_ptr<Text::Glyph>>& charMap,
      std::string text,
      glm::vec2 screenPosition = {})
      : screenPosition(screenPosition) {
    int32_t offset{};
    for (auto character : text) {
      if (charMap.find(character) != charMap.end()) {
        auto& glyph = charMap[character];
        characters.push_back({offset, static_cast<FT_ULong>(character)});
        offset += glyph->getAdvance();

      } else {
        offset += charMap.at('i')->getAdvance();
        characters.push_back({offset, 0});
      }
    }
  }
};

struct TextVertexPushData {
  glm::vec2 scale;
  glm::float32 padding0[2];
  glm::vec2 position;
  glm::float32 padding1[2];
};

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
struct Physics {};
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
  tinygltf::TinyGLTF modelLoader;
  std::unique_ptr<vka::CommandPool> transferCommandPool;
  std::unique_ptr<vka::CommandBuffer> transferCmd;
  std::unique_ptr<vka::Fence> transferFence;
  std::unique_ptr<vka::Swapchain> swapchain;
  std::unique_ptr<vka::ShaderModule> vertexShader;
  std::unique_ptr<vka::ShaderModule> fragmentShader;
  std::unique_ptr<vka::RenderPass> renderPass;
  std::vector<std::unique_ptr<vka::DescriptorSetLayout>> descriptorSetLayouts;
  std::unique_ptr<vka::PipelineLayout> pipelineLayout;
  std::unique_ptr<vka::PipelineCache> pipelineCache;
  std::unique_ptr<vka::GraphicsPipeline> pipeline;
  std::unique_ptr<Text::Library> textLibrary;
  struct TextData {
    std::unique_ptr<Text::Font> fontNiocTresni;
    std::map<FT_ULong, std::unique_ptr<Text::Glyph>> glyphMap;
    std::map<FT_ULong, size_t> indexBufferOffsets;
    std::unique_ptr<Text::Tileset> tilesetNiocTresni;
    std::unique_ptr<vka::Image> fontImage;
    std::unique_ptr<vka::ImageView> fontImageView;
    std::unique_ptr<vka::Sampler> fontSampler;
    std::unique_ptr<vka::Buffer> indexBuffer;
    std::unique_ptr<vka::Buffer> vertexBuffer;
    std::unique_ptr<vka::ShaderModule> vertexShader;
    std::unique_ptr<vka::ShaderModule> fragmentShader;
    std::unique_ptr<vka::DescriptorSetLayout> setLayout;
    std::unique_ptr<vka::DescriptorPool> descriptorPool;
    std::unique_ptr<vka::DescriptorSet> descriptorSet;
    std::unique_ptr<vka::PipelineLayout> pipelineLayout;
    std::unique_ptr<vka::GraphicsPipeline> pipeline;
    std::unique_ptr<TextObject> testText;
  } textData;

  asset::Collection shapesAsset;
  asset::Collection terrainAsset;
  std::vector<VkImage> swapImages;
  std::vector<std::unique_ptr<vka::ImageView>> swapImageViews;
  std::unique_ptr<vka::Image> depthImage;
  std::unique_ptr<vka::ImageView> depthImageView;

  struct BufferedState {
    std::unique_ptr<vka::DescriptorPool> descriptorPool;
    std::vector<std::unique_ptr<vka::DescriptorSet>> descriptorSets;
    vka::vulkan_vector<Material, vka::StorageBufferDescriptor> materialUniform;
    vka::vulkan_vector<Light, vka::StorageBufferDescriptor>
        dynamicLightsUniform;
    vka::vulkan_vector<LightData> lightDataUniform;
    vka::vulkan_vector<Camera> cameraUniform;
    vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor> instanceUniform;
    std::unique_ptr<vka::Fence> frameAcquired;
    std::unique_ptr<vka::Fence> bufferExecuted;
    std::unique_ptr<vka::Semaphore> renderComplete;
    uint32_t swapImageIndex;
    vka::UniqueFramebuffer framebuffer;
    std::unique_ptr<vka::CommandPool> commandPool;
    std::unique_ptr<vka::CommandBuffer> cmd;
    entt::DefaultRegistry ecs;
  };
  std::array<BufferedState, vka::BufferCount> bufState;

  asset::Collection loadCollection(const std::string& assetPath) {
    tinygltf::Model gltfModel;
    std::string loadWarning;
    std::string loadError;
    auto loadResult = modelLoader.LoadASCIIFromFile(
        &gltfModel, &loadError, &loadWarning, assetPath);
    if (!loadResult) {
      MultiLogger::get()->error(
          "Error while loading {}: {}", assetPath, loadError);
    }
    asset::Collection result;
    auto nodeIndex = 0U;
    for (auto& node : gltfModel.nodes) {
      asset::Model model{};
      model.name = node.name;
      auto primitive = gltfModel.meshes[node.mesh].primitives.at(0);
      auto indexAccessor = gltfModel.accessors[primitive.indices];
      auto positionAccessor =
          gltfModel.accessors[primitive.attributes["POSITION"]];
      auto normalAccessor = gltfModel.accessors[primitive.attributes["NORMAL"]];
      auto indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
      auto positionBufferView =
          gltfModel.bufferViews[positionAccessor.bufferView];
      auto normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
      model.indexByteOffset = indexBufferView.byteOffset;
      model.indexCount = indexAccessor.count;
      model.positionByteOffset = positionBufferView.byteOffset;
      model.normalByteOffset = normalBufferView.byteOffset;
      result.models[nodeIndex] = std::move(model);
      ++nodeIndex;
    }
    auto bufferSize = gltfModel.buffers.at(0).data.size();
    result.buffer = device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        true);
    auto stagingBuffer = device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        false);
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *stagingBuffer, "ModelVertexIndexStaging");

    auto copyFence = device->createFence(false);

    void* stagePtr{};
    vmaMapMemory(device->getAllocator(), *stagingBuffer, &stagePtr);
    std::memcpy(stagePtr, gltfModel.buffers[0].data.data(), bufferSize);
    vmaFlushAllocation(device->getAllocator(), *stagingBuffer, 0, bufferSize);
    auto cmdPool = device->createCommandPool();
    auto cmd = cmdPool->allocateCommandBuffer();
    cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmd->copyBuffer(*stagingBuffer, *result.buffer, {{0U, 0U, bufferSize}});
    cmd->end();
    device->queueSubmit({}, {*cmd}, {}, *copyFence);
    copyFence->wait();
    return result;
  }

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

  void pipeline3DRender(uint32_t renderIndex, VkExtent2D swapExtent) {
    auto& render = bufState[renderIndex];

    auto viewport = VkViewport{0,
                               0,
                               static_cast<float>(swapExtent.width),
                               static_cast<float>(swapExtent.height),
                               0,
                               1};

    render.cmd->setViewport(0, {viewport});
    render.cmd->setScissor(
        0, {{{0, 0}, {swapExtent.width, swapExtent.height}}});
    render.cmd->bindGraphicsPipeline(*pipeline);
    render.cmd->bindGraphicsDescriptorSets(
        *pipelineLayout,
        0,
        {*render.descriptorSets[0],
         *render.descriptorSets[1],
         *render.descriptorSets[2],
         *render.descriptorSets[3],
         *render.descriptorSets[4]},
        {0});
    auto someModel = shapesAsset.models[0];
    render.cmd->bindIndexBuffer(
        *shapesAsset.buffer, someModel.indexByteOffset, VK_INDEX_TYPE_UINT16);
    render.cmd->bindVertexBuffers(
        0,
        {*shapesAsset.buffer, *shapesAsset.buffer},
        {someModel.positionByteOffset, someModel.normalByteOffset});
    // render.cmd->drawIndexed(someModel.indexCount, 1, 0, 0, 0);
    uint32_t matIndex{};
    render.cmd->pushConstants(
        *pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &matIndex);
    auto terrainBuffer = *terrainAsset.buffer;
    render.cmd->bindIndexBuffer(
        terrainBuffer,
        terrainAsset.models[0].indexByteOffset,
        VK_INDEX_TYPE_UINT16);
    render.cmd->bindVertexBuffers(
        0,
        {terrainBuffer, terrainBuffer},
        {terrainAsset.models[0].positionByteOffset,
         terrainAsset.models[0].normalByteOffset});
    render.cmd->drawIndexed(terrainAsset.models[0].indexCount, 1, 0, 0, 0);
  }

  void pipelineTextRender(uint32_t renderIndex, VkExtent2D swapExtent) {
    auto& render = bufState[renderIndex];
    auto viewport = VkViewport{0,
                               0,
                               static_cast<float>(swapExtent.width),
                               static_cast<float>(swapExtent.height),
                               0,
                               1};
    auto scissor = VkRect2D{{0, 0}, {swapExtent.width, swapExtent.height}};
    render.cmd->bindGraphicsPipeline(*textData.pipeline);
    render.cmd->bindGraphicsDescriptorSets(
        *textData.pipelineLayout, 0, {*textData.descriptorSet}, {});
    render.cmd->bindIndexBuffer(*textData.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    render.cmd->bindVertexBuffers(0, {*textData.vertexBuffer}, {0});
    render.cmd->setViewport(0, {viewport});
    render.cmd->setScissor(0, {scissor});
    TextVertexPushData pushData{};
    pushData.scale = {1.f / swapExtent.width, 1.f / swapExtent.height};
    for (auto& charData : textData.testText->characters) {
      pushData.position =
          textData.testText->screenPosition + glm::vec2(charData.xOffset, 0.f);
      render.cmd->pushConstants(
          *textData.pipelineLayout,
          VK_SHADER_STAGE_VERTEX_BIT,
          0,
          sizeof(TextVertexPushData),
          &pushData);
      render.cmd->drawIndexed(
          6, 1, textData.indexBufferOffsets[charData.character], 0, 0);
    }
  }

  void renderCallback(vka::Engine* engine) {
    auto renderIndex = engine->currentRenderIndex();
    auto& render = bufState[renderIndex];

    if (auto index = swapchain->acquireImage(*render.frameAcquired)) {
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
    render.frameAcquired->wait();
    render.frameAcquired->reset();
    render.bufferExecuted->wait();
    render.bufferExecuted->reset();
    for (auto& set : render.descriptorSets) {
      set->validate(*device);
    }
    render.commandPool.reset();
    // auto draw_data = prepareImguiRender(render.swapImageIndex);
    auto swapExtent = swapchain->getSwapExtent();
    if (swapExtent.width == 0 || swapExtent.height == 0) {
      return;
    }
    render.framebuffer = device->createFramebuffer(
        {*swapImageViews[render.swapImageIndex], *depthImageView},
        *renderPass,
        swapExtent.width,
        swapExtent.height);
    render.cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    std::vector<VkClearValue> clearValues = {
        VkClearValue{{{0.f, 0.f, 0.f, 1.f}}}, VkClearValue{{{1.f, 0U}}}};

    render.cmd->beginRenderPass(
        *renderPass,
        render.framebuffer.get(),
        {{0, 0}, swapExtent},
        clearValues,
        VK_SUBPASS_CONTENTS_INLINE);

    pipeline3DRender(renderIndex, swapExtent);

    render.cmd->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);

    pipelineTextRender(renderIndex, swapExtent);

    render.cmd->endRenderPass();
    render.cmd->end();
    device->queueSubmit(
        {}, {*render.cmd}, {*render.renderComplete}, *render.bufferExecuted);
    auto presentResult = device->presentImage(
        *swapchain, render.swapImageIndex, *render.renderComplete);

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
    swapImages = swapchain->getSwapImages();
    for (const auto& swapImage : swapImages) {
      swapImageViews.push_back(device->createImageView2D(
          swapImage, swapFormat, vka::ImageAspect::Color));
    }
    depthImage = device->createImage2D(
        swapchain->getSwapExtent(),
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        vka::ImageAspect::Depth,
        true);
    depthImageView = device->createImageView2D(
        *depthImage, depthFormat, vka::ImageAspect::Depth);
  }

  template <typename T>
  std::unique_ptr<vka::Buffer>
  recordBufferUpload(gsl::span<T> data, VkBuffer buffer, VkDeviceSize offset) {
    auto staging = device->createBuffer(
        data.size_bytes(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        false);
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    VkBufferCopy bufferCopy{0, offset, data.size_bytes()};
    transferCmd->copyBuffer(*staging, buffer, {bufferCopy});
    return staging;
  }

  template <typename T>
  std::unique_ptr<vka::Buffer> recordImageUpload(
      gsl::span<T> data,
      VkImage image,
      VkOffset2D imageOffset,
      VkExtent2D imageExtent) {
    auto staging = device->createBuffer(
        data.size_bytes(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        false);
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    staging->unmap();

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
    transferCmd->pipelineBarrier(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        {},
        {},
        {preCopyBarrier});

    VkBufferImageCopy copy{};
    copy.imageOffset = {imageOffset.x, imageOffset.y, 0};
    copy.imageExtent = {imageExtent.width, imageExtent.height, 1};
    copy.imageSubresource = imageSubresource;
    transferCmd->copyBufferToImage(
        *staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy});

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
    transferCmd->pipelineBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        {},
        {},
        {postCopyBarrier});
    return staging;
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

    textLibrary = std::make_unique<Text::Library>();

    textData.fontNiocTresni =
        textLibrary->loadFont("content/fonts/NiocTresni/NiocTresni.ttf");
    auto niocFace = textData.fontNiocTresni->createFace(0);
    niocFace->setSize(32, 72);
    textData.glyphMap = niocFace->getGlyphs();
    textData.testText = std::make_unique<TextObject>(
        textData.glyphMap, std::string{"Test Text!"}, glm::vec2(50.f, 50.f));
    Text::Dimensions tileDimensions{};
    RANGES_FOR(const auto& glyphPtr, ranges::view::values(textData.glyphMap)) {
      auto glyphDimensions = glyphPtr->getDimensions();
      tileDimensions.width =
          std::max(glyphDimensions.width, tileDimensions.width);
      tileDimensions.height =
          std::max(glyphDimensions.height, tileDimensions.height);
    };
    std::vector<Text::Glyph*> glyphs;
    ranges::action::push_back(
        glyphs,
        ranges::view::transform(
            ranges::view::values(textData.glyphMap),
            [](auto& glyphPtr) { return glyphPtr.get(); }));
    std::vector<Text::Tile> tiles;
    ranges::action::push_back(
        tiles, ranges::view::transform(glyphs, [](auto& glyph) {
          return glyph->getTile();
        }));

    textData.tilesetNiocTresni = std::make_unique<Text::Tileset>(
        tiles, 2048, tileDimensions.width, tileDimensions.height);
    textData.fontImage = device->createImage2D(
        {static_cast<uint32_t>(textData.tilesetNiocTresni->width),
         static_cast<uint32_t>(textData.tilesetNiocTresni->height)},
        VK_FORMAT_R8G8B8A8_UINT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        vka::ImageAspect::Color);
    textData.fontImageView = device->createImageView2D(
        *textData.fontImage, VK_FORMAT_R8G8B8A8_UINT, vka::ImageAspect::Color);
    textData.fontSampler = device->createSampler();
    textData.descriptorPool = device->createDescriptorPool(
        {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}}, 1);
    textData.setLayout =
        device->createSetLayout({{0,
                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  1,
                                  VK_SHADER_STAGE_FRAGMENT_BIT,
                                  *textData.fontSampler}});

    textData.descriptorSet = textData.descriptorPool->allocateDescriptorSet(
        textData.setLayout.get());

    auto fontImageDescriptor =
        textData.descriptorSet->getDescriptor<vka::ImageSamplerDescriptor>(
            vka::DescriptorReference{});
    (*fontImageDescriptor)(
        *textData.fontImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    textData.descriptorSet->validate(*device);

    textData.pipelineLayout = device->createPipelineLayout(
        {VkPushConstantRange{
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TextVertexPushData)}},
        {*textData.setLayout});

    textData.vertexShader =
        device->createShaderModule("content/shaders/text.vert.spv");
    textData.fragmentShader =
        device->createShaderModule("content/shaders/text.frag.spv");

    std::vector<TextIndex> textIndices;
    std::vector<TextVertex> textVertices;
    size_t indexOffset{};
    size_t vertexOffset{};
    RANGES_FOR(
        const auto& zipped,
        ranges::view::zip(
            textData.glyphMap, textData.tilesetNiocTresni->tileUVs)) {
      const auto& glyphPair = std::get<0>(zipped);
      const auto& uv = std::get<1>(zipped);

      std::vector<TextIndex> nextIndices{
          static_cast<TextIndex>(vertexOffset + 0),
          static_cast<TextIndex>(vertexOffset + 1),
          static_cast<TextIndex>(vertexOffset + 2),
          static_cast<TextIndex>(vertexOffset + 2),
          static_cast<TextIndex>(vertexOffset + 3),
          static_cast<TextIndex>(vertexOffset + 0)};
      ranges::action::push_back(textIndices, std::move(nextIndices));
      textData.indexBufferOffsets[glyphPair.first] = indexOffset;

      auto glyphBounds = glyphPair.second->getBoundingBox();
      std::vector<TextVertex> nextVertices{
          {glm::vec2(glyphBounds.xmin, glyphBounds.ymax),
           glm::vec2(uv.xmin, uv.ymin)},
          {glm::vec2(glyphBounds.xmin, glyphBounds.ymin),
           glm::vec2(uv.xmin, uv.ymax)},
          {glm::vec2(glyphBounds.xmax, glyphBounds.ymin),
           glm::vec2(uv.xmax, uv.ymax)},
          {glm::vec2(glyphBounds.xmax, glyphBounds.ymax),
           glm::vec2(uv.xmax, uv.ymin)}};
      ranges::action::push_back(textVertices, std::move(nextVertices));
      indexOffset += 6;
      vertexOffset += 4;
    }

    transferCommandPool = device->createCommandPool();
    transferCmd = transferCommandPool->allocateCommandBuffer();
    transferFence = device->createFence(false);
    transferCmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    textData.indexBuffer = device->createBuffer(
        textIndices.size() * sizeof(TextIndex),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        true);
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *textData.indexBuffer, "TextIndexBuffer");
    textData.vertexBuffer = device->createBuffer(
        textVertices.size() * sizeof(TextVertex),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        true);
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *textData.vertexBuffer, "TextVertexBuffer");
    std::vector<std::unique_ptr<vka::Buffer>> stagingBuffers;

    stagingBuffers.push_back(
        recordBufferUpload<TextIndex>({textIndices}, *textData.indexBuffer, 0));
    stagingBuffers.push_back(recordBufferUpload<TextVertex>(
        {textVertices}, *textData.vertexBuffer, 0));

    for (size_t tileIndex{}; tileIndex < tiles.size(); ++tileIndex) {
      auto& tile = tiles[tileIndex];
      auto& pos = textData.tilesetNiocTresni->tileRects[tileIndex];
      auto glyph = glyphs[tileIndex];

      auto glyphDimensions = glyph->getDimensions();
      if (tile.size() == 0) {
        continue;
      }
      stagingBuffers.push_back(recordImageUpload(
          tile,
          *textData.fontImage,
          {pos.xmin, pos.ymin},
          {glyphDimensions.width, glyphDimensions.height}));
    }

    transferCmd->end();
    device->queueSubmit({}, {*transferCmd}, {}, *transferFence);
    transferFence->wait();
    stagingBuffers.clear();
    transferFence->reset();

    shapesAsset = loadCollection("content/models/shapes.gltf");
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *shapesAsset.buffer, "ShapesVertexBuffer");
    terrainAsset = loadCollection("content/models/terrain.gltf");
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *terrainAsset.buffer, "TerrainVertexBuffer");

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
    descriptorSetLayouts.push_back(device->createSetLayout({materialBinding}));
    descriptorSetLayouts.push_back(
        device->createSetLayout({dynamicLightBinding}));
    descriptorSetLayouts.push_back(device->createSetLayout({lightDataBinding}));
    descriptorSetLayouts.push_back(device->createSetLayout({cameraBinding}));
    descriptorSetLayouts.push_back(device->createSetLayout({instanceBinding}));

    for (auto& state : bufState) {
      MultiLogger::get()->info("creating command pool");
      state.commandPool = device->createCommandPool();
      state.cmd = state.commandPool->allocateCommandBuffer();

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
      for (auto& setLayout : descriptorSetLayouts) {
        state.descriptorSets.push_back(
            state.descriptorPool->allocateDescriptorSet(setLayout.get()));
      }

      state.materialUniform.subscribe(
          state.descriptorSets[0]->getDescriptor<vka::StorageBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      state.dynamicLightsUniform.subscribe(
          state.descriptorSets[1]->getDescriptor<vka::StorageBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      state.lightDataUniform.subscribe(
          state.descriptorSets[2]->getDescriptor<vka::BufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      state.cameraUniform.subscribe(
          state.descriptorSets[3]->getDescriptor<vka::BufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      state.instanceUniform.subscribe(
          state.descriptorSets[4]->getDescriptor<vka::DynamicBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));

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
        {*descriptorSetLayouts[0],
         *descriptorSetLayouts[1],
         *descriptorSetLayouts[2],
         *descriptorSetLayouts[3],
         *descriptorSetLayouts[4]});

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
    auto subpassText = renderPassCreateInfo.addGraphicsSubpass();
    subpassText->addColorRef(
        {colorAttachmentDesc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    renderPassCreateInfo.addSubpassDependency(
        subpass3D,
        subpassText,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        VK_DEPENDENCY_BY_REGION_BIT);

    MultiLogger::get()->info("creating render pass");
    renderPass = device->createRenderPass(renderPassCreateInfo);

    MultiLogger::get()->info("creating pipeline cache");
    pipelineCache = device->createPipelineCache();

    vka::GraphicsPipelineCreateInfo textPipelineInfo{
        *textData.pipelineLayout, *renderPass, 1};
    textPipelineInfo.addColorBlendAttachment(
        true,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    textPipelineInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    textPipelineInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    textPipelineInfo.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT,
        {},
        0,
        nullptr,
        *textData.vertexShader,
        "main");
    textPipelineInfo.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT,
        {},
        0,
        nullptr,
        *textData.fragmentShader,
        "main");
    textPipelineInfo.addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
    textPipelineInfo.addVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, 8);
    textPipelineInfo.addVertexBinding(
        0, sizeof(TextVertex), VK_VERTEX_INPUT_RATE_VERTEX);
    textPipelineInfo.addViewportScissor({}, {});
    textPipelineInfo.setCullMode(VK_CULL_MODE_NONE);

    textData.pipeline =
        device->createGraphicsPipeline(*pipelineCache, textPipelineInfo);

    auto pipeline3DInfo =
        vka::GraphicsPipelineCreateInfo(*pipelineLayout, *renderPass, 0);
    pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipeline3DInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipeline3DInfo.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT, {}, 0, nullptr, *vertexShader, "main");
    FragmentSpecData fragmentSpecData{1, 1};
    pipeline3DInfo.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT,
        {{0, 0, 4}, {1, 4, 4}},
        sizeof(FragmentSpecData),
        &fragmentSpecData,
        *fragmentShader,
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

    MultiLogger::get()->info("creating pipeline");
    pipeline = device->createGraphicsPipeline(*pipelineCache, pipeline3DInfo);

    engine->run();
  }
};

int main() {
  AppState appState{};

  return 0;
}