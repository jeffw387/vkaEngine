#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <cstring>
#include <mutex>
#include <vector>
#include <experimental/filesystem>
#include <range/v3/all.hpp>
#include <fstream>
#include <string>
#include "entt/entt.hpp"
#include "Engine.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "Camera.hpp"
#include "Asset.hpp"
#include "vulkan_vector.hpp"
#include "VkEnumStrings.hpp"
#include "Text.hpp"
#define THSVS_SIMPLER_VULKAN_SYNCHRONIZATION_IMPLEMENTATION
#include "thsvs_simpler_vulkan_synchronization.h"
#include "Input.hpp"

#include "test-text.hpp"
#include "test-debug.hpp"
#include "test-transfer.hpp"
#include "test-p3d-pipeline.hpp"

namespace fs = std::experimental::filesystem;
namespace view = ranges::view;
namespace action = ranges::action;
namespace Components {

struct Mesh {
  size_t meshIndex;
};
struct Material {
  uint32_t materialIndex;
};
struct Physics {};
}  // namespace Components

struct PolySize {
  const size_t size;
  template <typename T>
  operator T() {
    return static_cast<T>(size);
  }
};

struct Swap {
  VkFormat swapFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  std::unique_ptr<vka::Swapchain> swapchain;
  std::vector<VkImage> images;
  std::vector<std::shared_ptr<vka::ImageView>> views;
  std::shared_ptr<vka::Image> depthImage;
  std::shared_ptr<vka::ImageView> depthView;

  Swap() {}
  Swap(vka::Device* device)
      : swapchain(device->createSwapchain()),
        images(swapchain->getSwapImages()),
        views([&]() {
          std::vector<std::shared_ptr<vka::ImageView>> result;
          for (auto& image : images) {
            result.push_back(std::make_shared<vka::ImageView>(
                *device, image, swapFormat, vka::ImageAspect::Color));
          }
          return result;
        }()),
        depthImage(device->createImage2D(
            swapchain->getSwapExtent(),
            depthFormat,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            vka::ImageAspect::Depth,
            true)),
        depthView(device->createImageView2D(
            depthImage,
            depthFormat,
            vka::ImageAspect::Depth)) {}
};

struct BufferedState {
  std::unique_ptr<vka::DescriptorPool> descriptorPool;
  std::shared_ptr<vka::DescriptorSet> materialSet;
  std::shared_ptr<vka::DescriptorSet> dynamicLightSet;
  std::shared_ptr<vka::DescriptorSet> lightDataSet;
  std::shared_ptr<vka::DescriptorSet> cameraSet;
  std::shared_ptr<vka::DescriptorSet> instanceSet;
  vka::StorageBufferDescriptor* materialDescriptor;
  vka::StorageBufferDescriptor* dynamicLightDescriptor;
  vka::BufferDescriptor* lightDataDescriptor;
  vka::BufferDescriptor* cameraDescriptor;
  vka::DynamicBufferDescriptor* instanceDescriptor;
  std::unique_ptr<vka::vulkan_vector<Material, vka::StorageBufferDescriptor>>
      materialUniform;
  std::unique_ptr<vka::vulkan_vector<Light, vka::StorageBufferDescriptor>>
      dynamicLightsUniform;
  std::unique_ptr<vka::vulkan_vector<LightData>> lightDataUniform;
  std::unique_ptr<vka::vulkan_vector<Camera>> cameraUniform;
  std::unique_ptr<vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor>>
      instanceUniform;
  std::unique_ptr<vka::Fence> frameAcquired;
  std::unique_ptr<vka::Fence> bufferExecuted;
  std::unique_ptr<vka::Semaphore> renderComplete;
  uint32_t swapImageIndex = 0;
  std::shared_ptr<vka::Framebuffer> framebuffer;
  std::unique_ptr<vka::CommandPool> commandPool;
  std::weak_ptr<vka::CommandBuffer> cmd;
  entt::DefaultRegistry ecs;
  std::unordered_map<Input::InputEvent, bool> inputState;
  BufferedState() {}
  BufferedState(
      vka::Device* device,
      vka::DescriptorSetLayout* materialLayout,
      vka::DescriptorSetLayout* dynamicLightLayout,
      vka::DescriptorSetLayout* lightDataLayout,
      vka::DescriptorSetLayout* cameraLayout,
      vka::DescriptorSetLayout* instanceLayout)
      : descriptorPool(device->createDescriptorPool(
            {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3},
             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3}},
            5)),
        materialSet(descriptorPool->allocateDescriptorSet(materialLayout)),
        dynamicLightSet(
            descriptorPool->allocateDescriptorSet(dynamicLightLayout)),
        lightDataSet(descriptorPool->allocateDescriptorSet(lightDataLayout)),
        cameraSet(descriptorPool->allocateDescriptorSet(cameraLayout)),
        instanceSet(descriptorPool->allocateDescriptorSet(instanceLayout)),
        materialDescriptor(
            materialSet->getDescriptor<vka::StorageBufferDescriptor>(
                {{}, 0, 0})),
        dynamicLightDescriptor(
            dynamicLightSet->getDescriptor<vka::StorageBufferDescriptor>(
                {{}, 0, 0})),
        lightDataDescriptor(
            lightDataSet->getDescriptor<vka::BufferDescriptor>({{}, 0, 0})),
        cameraDescriptor(
            cameraSet->getDescriptor<vka::BufferDescriptor>({{}, 0, 0})),
        instanceDescriptor(
            instanceSet->getDescriptor<vka::DynamicBufferDescriptor>(
                {{}, 0, 0})),
        materialUniform(
            std::make_unique<
                vka::vulkan_vector<Material, vka::StorageBufferDescriptor>>(
                device,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                std::vector{materialDescriptor})),
        dynamicLightsUniform(
            std::make_unique<
                vka::vulkan_vector<Light, vka::StorageBufferDescriptor>>(
                device,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                std::vector{dynamicLightDescriptor})),
        lightDataUniform(std::make_unique<vka::vulkan_vector<LightData>>(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            std::vector{lightDataDescriptor})),
        cameraUniform(std::make_unique<vka::vulkan_vector<Camera>>(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            std::vector{cameraDescriptor})),
        instanceUniform(
            std::make_unique<
                vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor>>(
                device,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                std::vector{instanceDescriptor})),
        frameAcquired(device->createFence(false)),
        bufferExecuted(device->createFence(true)),
        renderComplete(device->createSemaphore()),
        commandPool(device->createCommandPool()),
        cmd(commandPool->allocateCommandBuffer()) {}
};

struct Assets {
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
    result.data = gltfModel.buffers.at(0).data;

    return result;
  }

  tinygltf::TinyGLTF modelLoader;
  asset::Collection shapes;
  asset::Collection terrain;

  Assets(vka::Device* device, Transfer* transfer)
      : shapes{loadCollection("content/models/shapes.gltf")},
        terrain{loadCollection("content/models/terrain.gltf")} {}
};

auto createRenderPass = [](vka::Device* device,
                           VkFormat swapFormat,
                           VkFormat depthFormat) {
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
  subpass3D->setDepthRef(
      {depthAttachmentDesc, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
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

  return device->createRenderPass(renderPassCreateInfo);
};

auto createBufferedStates = [](vka::Device* device, P3DPipeline* pipeline) {
  std::array<BufferedState, vka::BufferCount> result;
  for (auto& state : result) {
    state = BufferedState(
        device,
        pipeline->materialLayout.get(),
        pipeline->dynamicLightLayout.get(),
        pipeline->lightDataLayout.get(),
        pipeline->cameraLayout.get(),
        pipeline->instanceLayout.get());
  }
  return result;
};

struct AppState {
  PolySize defaultWidth = PolySize{900U};
  PolySize defaultHeight = PolySize{900U};

  vka::OrthoCamera mainCamera;
  vka::EngineCreateInfo engineCreateInfo;
  std::unique_ptr<vka::Engine> engine;
  vka::InstanceCreateInfo instanceCreateInfo;
  vka::Instance* instance;
  vka::SurfaceCreateInfo surfaceCreateInfo;
  vka::Surface* surface;
  vka::Device* device;
  Transfer transfer;
  std::unique_ptr<Assets> assets;
  std::unique_ptr<Swap> swap;
  std::shared_ptr<vka::RenderPass> renderPass;

  std::unique_ptr<vka::PipelineCache> pipelineCache;
  float testValue = 0.004f;
  TextPipeline textPipeline;
  Font testFont;
  std::unique_ptr<TextObject> testText;
  vka::Clock::time_point lastRenderTime;
  std::unique_ptr<TextObject> fps_text;
  std::unique_ptr<TextObject> test_value_text;
  P3DPipeline p3DPipeline;

  std::array<BufferedState, vka::BufferCount> bufState;
  std::map<uint64_t, Input::InputEvent> inputBindings;

  void initCallback(vka::Engine* engine, int32_t initialIndex) {
    MultiLogger::get()->info("Init Callback");
    auto& initial = bufState[initialIndex];

    initial.materialUniform->push_back({glm::vec4(0.8f, 1.f, 0.8f, 1.f)});
    initial.materialUniform->flushMemory(device);

    initial.dynamicLightsUniform->push_back(
        {{0.8f, 0.8f, 0.8f, 25.f}, {0.f, 0.f, 2.f, 0.f}});
    initial.dynamicLightsUniform->flushMemory(device);

    LightData lightData;
    lightData.count = 1;
    lightData.ambient = glm::vec4(0.f, 0.f, 1.f, 10.0f);
    initial.lightDataUniform->push_back(std::move(lightData));
    initial.lightDataUniform->flushMemory(device);

    Camera camData{};
    camData.projection = mainCamera.getProjection();
    camData.view = mainCamera.getView();
    initial.cameraUniform->push_back(std::move(camData));
    initial.cameraUniform->flushMemory(device);

    initial.instanceUniform->push_back(
        {glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f))});
    initial.instanceUniform->flushMemory(device);
  }

  void updateCallback(vka::Engine* engine) {
    auto updateIndex = engine->currentUpdateIndex();
    auto lastUpdateIndex = engine->previousUpdateIndex();
    auto& last = bufState[lastUpdateIndex];
    auto& current = bufState[updateIndex];
    auto updateTime = engine->updateTimePoint(updateIndex);

    auto matSize = last.materialUniform->size();
    current.materialUniform->resize(matSize);
    for (auto i = 0U; i < matSize; ++i) {
      (*current.materialUniform)[i] = (*last.materialUniform)[i];
    }
    current.materialUniform->flushMemory(device);

    auto dynamicLightSize = last.dynamicLightsUniform->size();
    current.dynamicLightsUniform->resize(dynamicLightSize);
    for (auto i = 0U; i < dynamicLightSize; ++i) {
      (*current.dynamicLightsUniform)[i] = (*last.dynamicLightsUniform)[i];
    }
    current.dynamicLightsUniform->flushMemory(device);

    current.lightDataUniform->resize(1);
    (*current.lightDataUniform)[0] = (*last.lightDataUniform)[0];
    current.lightDataUniform->flushMemory(device);

    current.cameraUniform->resize(1);
    (*current.cameraUniform)[0].projection = mainCamera.getProjection();
    (*current.cameraUniform)[0].view = mainCamera.getView();
    current.cameraUniform->flushMemory(device);

    current.instanceUniform->copy_from(*last.instanceUniform);
    current.instanceUniform->flushMemory(device);
  }

  void pipeline3DRender(uint32_t renderIndex, VkExtent2D swapExtent) {
    auto& render = bufState[renderIndex];

    auto viewport = VkViewport{0,
                               0,
                               static_cast<float>(swapExtent.width),
                               static_cast<float>(swapExtent.height),
                               0,
                               1};

    if (auto cmd = render.cmd.lock()) {
      cmd->bindGraphicsPipeline(p3DPipeline.pipeline);
      cmd->setViewport(0, {viewport});
      cmd->setScissor(0, {{{0, 0}, {swapExtent.width, swapExtent.height}}});
      cmd->bindGraphicsDescriptorSets(
          p3DPipeline.pipelineLayout,
          0,
          {render.materialSet,
           render.dynamicLightSet,
           render.lightDataSet,
           render.cameraSet,
           render.instanceSet},
          {0});
      auto someModel = assets->shapes.models[0];
      cmd->bindIndexBuffer(
          assets->shapes.buffer,
          someModel.indexByteOffset,
          VK_INDEX_TYPE_UINT16);
      cmd->bindVertexBuffers(
          0,
          {assets->shapes.buffer, assets->shapes.buffer},
          {someModel.positionByteOffset, someModel.normalByteOffset});
      // cmd->drawIndexed(someModel.indexCount, 1, 0, 0, 0);
      uint32_t matIndex{};
      cmd->pushConstants(
          p3DPipeline.pipelineLayout,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          0,
          4,
          &matIndex);
      cmd->bindIndexBuffer(
          assets->terrain.buffer,
          assets->terrain.models[0].indexByteOffset,
          VK_INDEX_TYPE_UINT16);
      cmd->bindVertexBuffers(
          0,
          {assets->terrain.buffer, assets->terrain.buffer},
          {assets->terrain.models[0].positionByteOffset,
           assets->terrain.models[0].normalByteOffset});
      cmd->drawIndexed(assets->terrain.models[0].indexCount, 1, 0, 0, 0);
    }
  }

  void pipelineTextRender(uint32_t renderIndex, VkExtent2D swapExtent) {
    auto& render = bufState[renderIndex];
    auto viewport = VkViewport{0,
                               0,
                               static_cast<float>(swapExtent.width),
                               static_cast<float>(swapExtent.height),
                               0,
                               1};

    TextPushData pushData{};
    auto scissor = VkRect2D{{0, 0}, {swapExtent.width, swapExtent.height}};
    if (auto cmd = render.cmd.lock()) {
      cmd->bindGraphicsPipeline(textPipeline.pipeline);
      cmd->bindGraphicsDescriptorSets(
          textPipeline.pipelineLayout, 0, {testFont.descriptorSet}, {});
      cmd->bindIndexBuffer(testFont.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
      cmd->bindVertexBuffers(0, {testFont.vertexBuffer}, {0});
      cmd->setViewport(0, {viewport});
      cmd->setScissor(0, {scissor});
      auto halfExtent =
          glm::vec2((float)swapExtent.width, (float)swapExtent.height) * 0.5f;
      pushData.vertex.clipSpaceScale = {2.f / swapExtent.width,
                                        2.f / swapExtent.height};
      auto renderText = [&](auto& currentText) {
        auto currentFont = currentText->font;
        auto currentScale =
            static_cast<float>(currentText->pixelHeight) /
            static_cast<float>(currentFont->getOriginalPixelHeight());
        constexpr auto AntiAliasFactor = 0.002f;
        constexpr auto AntiAliasOffset = 0.001f;
        auto msdfScale = AntiAliasFactor * currentScale + AntiAliasOffset;
        pushData.fragment.distanceFactor = msdfScale;
        auto& currentString = currentText->str;
        pushData.fragment.fontColor = currentText->color;
        pushData.vertex.textScale = currentScale;

        glm::vec2 pen = currentText->screenPosition;
        for (int i{}; i < currentString.size(); ++i) {
          auto currentCharCode = currentString[i];
          int currentGlyph = currentFont->getGlyphIndex(currentCharCode);
          try {
            pushData.fragment.glyphIndex =
                currentFont->getArrayIndex(currentGlyph);
          } catch (std::exception e) {
            MultiLogger::get()->error(
                "Exception thrown getting glyph array index: {}", e.what());
          }
          int nextGlyph{-1};
          float kerning{};
          if ((i + 1) < currentString.size()) {
            nextGlyph = currentFont->getGlyphIndex(currentString[i + 1]);
            kerning = currentFont->getKerning(
                currentGlyph, nextGlyph, currentText->pixelHeight);
          }
          float advanceX =
              currentFont->getAdvance(currentGlyph, currentText->pixelHeight);
          pushData.vertex.screenPos = pen - halfExtent;
          cmd->pushConstants(
              textPipeline.pipelineLayout,
              VK_SHADER_STAGE_VERTEX_BIT,
              offsetof(TextPushData, vertex),
              sizeof(pushData.vertex),
              &pushData.vertex);
          cmd->pushConstants(
              textPipeline.pipelineLayout,
              VK_SHADER_STAGE_FRAGMENT_BIT,
              offsetof(TextPushData, fragment),
              sizeof(pushData.fragment),
              &pushData.fragment);
          cmd->drawIndexed(
              Text::IndicesPerQuad,
              1,
              testFont.vertexData->offsets[currentGlyph],
              0,
              0);
          auto kernedAdvance = (advanceX + kerning) * currentScale *
                               currentFont->getScaleFactor();
          pen.x += kernedAdvance;
        }
      };
      renderText(testText);
      renderText(fps_text);
      if (glfwGetKey(*surface, GLFW_KEY_LEFT)) {
        testValue -= 0.0005f;
      } else if (glfwGetKey(*surface, GLFW_KEY_RIGHT)) {
        testValue += 0.0005f;
      }
      test_value_text->str = std::to_string(pushData.fragment.distanceFactor);
      renderText(test_value_text);
    }
  }

  void renderCallback(vka::Engine* engine) {
    auto renderIndex = engine->currentRenderIndex();
    auto& render = bufState[renderIndex];

    auto now = vka::Clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now - lastRenderTime)
                         .count();
    lastRenderTime = now;
    fps_text->str = std::to_string(frameTime) + "ms";

    if (auto index = swap->swapchain->acquireImage(*render.frameAcquired)) {
      render.swapImageIndex = index.value();
    } else {
      switch (index.error()) {
        case VK_NOT_READY:
          return;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
          device->waitIdle();
          swap.reset();
          swap = std::make_unique<Swap>(device);
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
    render.materialSet->validate(*device);
    render.dynamicLightSet->validate(*device);
    render.lightDataSet->validate(*device);
    render.cameraSet->validate(*device);
    render.instanceSet->validate(*device);
    render.commandPool->reset();

    auto swapExtent = swap->swapchain->getSwapExtent();
    if (swapExtent.width == 0 || swapExtent.height == 0) {
      return;
    }
    render.framebuffer = device->createFramebuffer(
        *renderPass,
        {swap->views[render.swapImageIndex], swap->depthView},
        swapExtent);

    if (auto cmd = render.cmd.lock()) {
      cmd->begin();

      std::vector<VkClearValue> clearValues = {
          VkClearValue{{{0.f, 0.f, 0.f, 1.f}}}, VkClearValue{{{1.f, 0U}}}};

      cmd->beginRenderPass(
          renderPass,
          render.framebuffer,
          {{0, 0}, swapExtent},
          clearValues,
          VK_SUBPASS_CONTENTS_INLINE);

      pipeline3DRender(renderIndex, swapExtent);

      cmd->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);

      pipelineTextRender(renderIndex, swapExtent);

      cmd->endRenderPass();
      cmd->end();
      device->queueSubmit(
          {}, {cmd}, {*render.renderComplete}, render.bufferExecuted.get());
    }
    auto presentResult = device->presentImage(
        *swap->swapchain, render.swapImageIndex, *render.renderComplete);

    switch (presentResult) {
      case VK_SUCCESS:
        return;
      case VK_ERROR_OUT_OF_DATE_KHR:
      case VK_SUBOPTIMAL_KHR:
        device->waitIdle();
        swap.reset();
        swap = std::make_unique<Swap>(device);
        return;
      default:
        MultiLogger::get()->critical(
            "Unrecoverable vulkan error: {}", vka::Results[presentResult]);
        throw std::runtime_error("Unrecoverable vulkan error.");
    }
  }

  void uploadData(std::shared_ptr<vka::CommandBuffer> cmd, vka::Fence* fence) {
    cmd->begin();
    size_t indexSize =
        testFont.vertexData->indices.size() * sizeof(Text::Index);
    auto indexStaging = device->createBuffer(
        indexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        true);
    size_t vertexSize =
        testFont.vertexData->vertices.size() * sizeof(Text::Vertex);
    auto vertexStaging = device->createBuffer(
        vertexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        true);
    auto pixelSpan = gsl::span<uint8_t>{testFont.pixels};
    auto textureStaging = device->createBuffer(
        pixelSpan.length_bytes(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        true);
    cmd->recordBufferUpload<Text::Index>(
        {testFont.vertexData->indices}, indexStaging, testFont.indexBuffer, 0);
    cmd->recordBufferUpload<Text::Vertex>(
        {testFont.vertexData->vertices},
        vertexStaging,
        testFont.vertexBuffer,
        0);
    cmd->recordImageBarrier(
        {},
        {THSVS_ACCESS_TRANSFER_WRITE},
        testFont.image,
        THSVS_IMAGE_LAYOUT_OPTIMAL,
        true);
    cmd->recordImageArrayUpload<uint8_t>(
        pixelSpan, textureStaging, testFont.image);
    cmd->recordImageBarrier(
        {THSVS_ACCESS_TRANSFER_WRITE},
        {THSVS_ACCESS_FRAGMENT_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER},
        testFont.image,
        THSVS_IMAGE_LAYOUT_OPTIMAL);

    auto uploadCollection = [&](asset::Collection& collection) {
      auto bufferSize = collection.data.size();
      collection.buffer = device->createBuffer(
          bufferSize,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
              VK_BUFFER_USAGE_TRANSFER_DST_BIT,
          VMA_MEMORY_USAGE_GPU_ONLY,
          true);
      auto stagingBuffer = device->createBuffer(
          bufferSize,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VMA_MEMORY_USAGE_CPU_ONLY,
          false);

      void* stagePtr = stagingBuffer->map();
      std::memcpy(stagePtr, collection.data.data(), bufferSize);
      stagingBuffer->flush();

      cmd->copyBuffer(stagingBuffer, collection.buffer, {{0U, 0U, bufferSize}});
      return stagingBuffer;
    };
    auto shapesStaging = uploadCollection(assets->shapes);
    auto terrainStaging = uploadCollection(assets->terrain);
    cmd->end();
    device->queueSubmit({}, {cmd}, {}, fence);
    fence->wait();
    fence->reset();
  }

  AppState()
      : defaultWidth{900U},
        defaultHeight{900U},
        mainCamera{},
        engineCreateInfo{[this](auto engine, auto initialIndex) {
                           initCallback(engine, initialIndex);
                         },
                         [this](auto engine) { updateCallback(engine); },
                         [this](auto engine) { renderCallback(engine); }},
        engine{std::make_unique<vka::Engine>(engineCreateInfo)},
        instanceCreateInfo{"testmain",
                           {0, 0, 1},
                           std::vector{"VK_KHR_surface", "VK_EXT_debug_utils"},
                           std::vector{"VK_LAYER_LUNARG_standard_validation"}},
        instance{engine->createInstance(instanceCreateInfo)},
        surfaceCreateInfo{defaultWidth, defaultHeight, "testmain window"},
        surface{instance->createSurface(surfaceCreateInfo)},
        device{instance->createDevice(
            {"VK_KHR_swapchain"},
            {},
            [&](const vka::PhysicalDeviceData& data) {
              for (const auto& [physicalDevice, props] : data.properties) {
                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                  return physicalDevice;
                }
              }
              return data.physicalDevices.at(0);
            })},
        transfer{device},
        assets{std::make_unique<Assets>(device, &transfer)},
        swap{std::make_unique<Swap>(device)},
        renderPass{
            createRenderPass(device, swap->swapFormat, swap->depthFormat)},
        pipelineCache{device->createPipelineCache()},
        textPipeline{device, renderPass.get(), pipelineCache.get()},
        testFont{"content/fonts/LiberationSans-Regular.ttf",
                 device,
                 textPipeline},
        testText{std::make_unique<TextObject>(
            glm::vec2(50.f, 100.f),
            std::string{"Test Text!"},
            100,
            testFont.font.get())},
        lastRenderTime{},
        fps_text{std::make_unique<TextObject>(
            glm::vec2(float(defaultWidth) - 100.f, 50.f),
            std::string{"NaN"},
            15,
            testFont.font.get())},
        test_value_text{std::make_unique<TextObject>(
            glm::vec2(float(defaultWidth) - 400.f, 100.f),
            std::to_string(testValue),
            60,
            testFont.font.get())},
        p3DPipeline{device, renderPass.get(), pipelineCache.get()},
        bufState{createBufferedStates(device, &p3DPipeline)} {
    mainCamera.setDimensions(2, 2);
    mainCamera.setPosition({0.f, 0.f, 0.f});
    mainCamera.setNearFar(-10, 10);

    if (auto cmd = transfer.cmd.lock()) {
      uploadData(cmd, transfer.fence.get());
    }

    engine->run();
  }
};

int main() {
  AppState appState{};

  return 0;
}