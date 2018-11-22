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

#include "test-text.hpp"
#include "test-debug.hpp"
#include "test-transfer.hpp"
#include "test-p3d-pipeline.hpp"

namespace fs = std::experimental::filesystem;

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
  Transfer transfer;
  std::shared_ptr<vka::RenderPass> renderPass;
  std::unique_ptr<vka::PipelineCache> pipelineCache;

  TextPipeline textPipeline;
  Font testFont;
  std::unique_ptr<TextObject> testText;

  P3DPipeline p3DPipeline;

  asset::Collection shapesAsset;
  asset::Collection terrainAsset;

  std::unique_ptr<vka::Swapchain> swapchain;
  std::vector<VkImage> swapImages;
  std::vector<std::shared_ptr<vka::ImageView>> swapImageViews;
  std::shared_ptr<vka::Image> depthImage;
  std::shared_ptr<vka::ImageView> depthImageView;

  struct BufferedState {
    std::unique_ptr<vka::DescriptorPool> descriptorPool;
    std::vector<std::shared_ptr<vka::DescriptorSet>> descriptorSets;
    vka::vulkan_vector<Material, vka::StorageBufferDescriptor> materialUniform;
    vka::vulkan_vector<Light, vka::StorageBufferDescriptor>
        dynamicLightsUniform;
    vka::vulkan_vector<LightData> lightDataUniform;
    vka::vulkan_vector<Camera> cameraUniform;
    vka::vulkan_vector<Instance, vka::DynamicBufferDescriptor> instanceUniform;
    std::unique_ptr<vka::Fence> frameAcquired;
    std::unique_ptr<vka::Fence> bufferExecuted;
    std::unique_ptr<vka::Semaphore> renderComplete;
    uint32_t swapImageIndex = 0;
    std::shared_ptr<vka::Framebuffer> framebuffer;
    std::unique_ptr<vka::CommandPool> commandPool;
    std::weak_ptr<vka::CommandBuffer> cmd;
    entt::DefaultRegistry ecs;
    BufferedState() {}
    BufferedState(
        vka::Device* device,
        const std::vector<std::unique_ptr<vka::DescriptorSetLayout>> layouts)
        : descriptorPool(device->createDescriptorPool(
              {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7},
               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3},
               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3}},
              5)),
          descriptorSets({
              descriptorPool->allocateDescriptorSet(layouts[0].get()),
              descriptorPool->allocateDescriptorSet(layouts[1].get()),
              descriptorPool->allocateDescriptorSet(layouts[2].get()),
              descriptorPool->allocateDescriptorSet(layouts[3].get()),
              descriptorPool->allocateDescriptorSet(layouts[4].get()),
          }),
          materialUniform({device,
                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           VMA_MEMORY_USAGE_CPU_TO_GPU}),
          dynamicLightsUniform({device,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                VMA_MEMORY_USAGE_CPU_TO_GPU}),
          lightDataUniform({device,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VMA_MEMORY_USAGE_CPU_TO_GPU}),
          cameraUniform({device,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VMA_MEMORY_USAGE_CPU_TO_GPU}),
          instanceUniform({device,
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VMA_MEMORY_USAGE_CPU_TO_GPU}),
          frameAcquired(device->createFence(false)),
          bufferExecuted(device->createFence(true)),
          renderComplete(device->createSemaphore()),
          commandPool(device->createCommandPool()),
          cmd(commandPool->allocateCommandBuffer()) {
      materialUniform.reserve(1);
      dynamicLightsUniform.reserve(1);
      lightDataUniform.reserve(1);
      cameraUniform.reserve(1);
      instanceUniform.reserve(1);

      materialUniform.subscribe(
          descriptorSets[0]->getDescriptor<vka::StorageBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      dynamicLightsUniform.subscribe(
          descriptorSets[1]->getDescriptor<vka::StorageBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      lightDataUniform.subscribe(
          descriptorSets[2]->getDescriptor<vka::BufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      cameraUniform.subscribe(
          descriptorSets[3]->getDescriptor<vka::BufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
      instanceUniform.subscribe(
          descriptorSets[4]->getDescriptor<vka::DynamicBufferDescriptor>(
              vka::DescriptorReference{VkDescriptorSet{}, 0, 0}));
    }
  };
  std::array<BufferedState, vka::BufferCount> bufState;

  void recordGlobalBarrier(
      std::vector<ThsvsAccessType> previous,
      std::vector<ThsvsAccessType> next) {
    ThsvsGlobalBarrier thBarrier{};
    thBarrier.prevAccessCount = static_cast<uint32_t>(previous.size());
    thBarrier.pPrevAccesses = previous.data();
    thBarrier.nextAccessCount = static_cast<uint32_t>(next.size());
    thBarrier.pNextAccesses = next.data();
    VkPipelineStageFlags src{};
    VkPipelineStageFlags dst{};
    VkMemoryBarrier barrier{};
    thsvsGetVulkanMemoryBarrier(thBarrier, &src, &dst, &barrier);

    if (auto cmd = transfer.cmd.lock()) {
      cmd->pipelineBarrier(src, dst, 0, {barrier}, {}, {});
    }
  }

  void recordImageBarrier(
      std::vector<ThsvsAccessType> previous,
      std::vector<ThsvsAccessType> next,
      std::shared_ptr<vka::Image> image,
      ThsvsImageLayout newLayout,
      bool discardContents = false) {
    ThsvsImageBarrier thBarrier{};
    thBarrier.discardContents = VkBool32(discardContents);
    thBarrier.prevAccessCount = static_cast<uint32_t>(previous.size());
    thBarrier.pPrevAccesses = previous.data();
    thBarrier.nextAccessCount = static_cast<uint32_t>(next.size());
    thBarrier.pNextAccesses = next.data();
    thBarrier.image = *image;
    thBarrier.prevLayout = image->layout;
    image->layout = newLayout;
    thBarrier.nextLayout = newLayout;
    thBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,
                                  0,
                                  VK_REMAINING_MIP_LEVELS,
                                  0,
                                  VK_REMAINING_ARRAY_LAYERS};
    VkPipelineStageFlags src{};
    VkPipelineStageFlags dst{};
    VkImageMemoryBarrier barrier{};
    thsvsGetVulkanImageMemoryBarrier(thBarrier, &src, &dst, &barrier);

    if (auto cmd = transfer.cmd.lock()) {
      cmd->pipelineBarrier(src, dst, 0, {}, {}, {barrier});
    }
  }

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

    void* stagePtr = stagingBuffer->map();
    std::memcpy(stagePtr, gltfModel.buffers[0].data.data(), bufferSize);
    stagingBuffer->flush();

    transfer.pool->reset();
    if (auto cmd = transfer.cmd.lock()) {
      cmd->begin();
      cmd->copyBuffer(stagingBuffer, result.buffer, {{0U, 0U, bufferSize}});
      cmd->end();
      device->queueSubmit({}, {cmd}, {}, copyFence.get());
    }
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

    if (auto cmd = render.cmd.lock()) {
      cmd->bindGraphicsPipeline(p3DPipeline.pipeline);
      cmd->setViewport(0, {viewport});
      cmd->setScissor(0, {{{0, 0}, {swapExtent.width, swapExtent.height}}});
      cmd->bindGraphicsDescriptorSets(
          p3DPipeline.pipelineLayout,
          0,
          {render.descriptorSets[0],
           render.descriptorSets[1],
           render.descriptorSets[2],
           render.descriptorSets[3],
           render.descriptorSets[4]},
          {0});
      auto someModel = shapesAsset.models[0];
      cmd->bindIndexBuffer(
          shapesAsset.buffer, someModel.indexByteOffset, VK_INDEX_TYPE_UINT16);
      cmd->bindVertexBuffers(
          0,
          {shapesAsset.buffer, shapesAsset.buffer},
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
          terrainAsset.buffer,
          terrainAsset.models[0].indexByteOffset,
          VK_INDEX_TYPE_UINT16);
      cmd->bindVertexBuffers(
          0,
          {terrainAsset.buffer, terrainAsset.buffer},
          {terrainAsset.models[0].positionByteOffset,
           terrainAsset.models[0].normalByteOffset});
      cmd->drawIndexed(terrainAsset.models[0].indexCount, 1, 0, 0, 0);
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
      pushData.vertex.clipSpaceScale = {1.f / swapExtent.width,
                                        1.f / swapExtent.height};
      pushData.fragment.clipSpaceScale = {1.f / swapExtent.width,
                                          1.f / swapExtent.height};
      auto& currentText = testText;
      pushData.fragment.fontColor = currentText->color;
      auto currentFont = currentText->font;
      pushData.vertex.textScale =
          currentFont->msdfToRenderRatio(currentText->pixelHeight);

      auto& currentString = currentText->str;
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
        pen.x += advanceX + kerning;
      }
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
    render.commandPool->reset();

    auto swapExtent = swapchain->getSwapExtent();
    if (swapExtent.width == 0 || swapExtent.height == 0) {
      return;
    }
    render.framebuffer = device->createFramebuffer(
        *renderPass,
        {swapImageViews[render.swapImageIndex], depthImageView},
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
      swapImageViews.push_back(std::make_shared<vka::ImageView>(
          *device, swapImage, swapFormat, vka::ImageAspect::Color));
    }
    depthImage = device->createImage2D(
        swapchain->getSwapExtent(),
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        vka::ImageAspect::Depth,
        true);
    depthImageView = device->createImageView2D(
        depthImage, depthFormat, vka::ImageAspect::Depth);
  }

  template <typename T>
  std::shared_ptr<vka::Buffer> recordBufferUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Buffer> buffer,
      VkDeviceSize offset) {
    auto staging = device->createBuffer(
        data.size_bytes(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        false);
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    VkBufferCopy bufferCopy{0, offset, data.size_bytes()};
    if (auto cmd = transfer.cmd.lock()) {
      cmd->copyBuffer(staging, buffer, {bufferCopy});
    }
    return staging;
  }

  template <typename T>
  std::shared_ptr<vka::Buffer> recordImageUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Image> image,
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

    if (auto cmd = transfer.cmd.lock()) {
      VkBufferImageCopy copy{};
      copy.imageOffset = {imageOffset.x, imageOffset.y, 0};
      copy.imageExtent = {imageExtent.width, imageExtent.height, 1};
      copy.imageSubresource = imageSubresource;
      cmd->copyBufferToImage(
          staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy});
    }
    return staging;
  }

  template <typename T>
  std::shared_ptr<vka::Buffer> recordImageArrayUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Image> image) {
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
    imageSubresource.layerCount = image->layerCount;
    imageSubresource.mipLevel = 0;

    if (auto cmd = transfer.cmd.lock()) {
      VkBufferImageCopy copy{};
      copy.imageOffset = {};
      copy.imageExtent = image->extent;
      copy.imageSubresource = imageSubresource;
      cmd->copyBufferToImage(
          staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy});
    }
    return staging;
  }

  void recordImageBlit(
      std::shared_ptr<vka::Image> source,
      std::shared_ptr<vka::Image> destination) {
    if (auto cmd = transfer.cmd.lock()) {
      VkImageBlit blit{};
      blit.srcSubresource.aspectMask =
          static_cast<VkImageAspectFlags>(vka::ImageAspect::Color);
      blit.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
      blit.dstSubresource.aspectMask =
          static_cast<VkImageAspectFlags>(vka::ImageAspect::Color);
      blit.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;

      blit.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
      cmd->blitImage(source, destination, {blit}, VK_FILTER_LINEAR);
    }
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

    constexpr auto fontPixelHeight = 60;

    testText = std::make_unique<TextObject>(
        glm::vec2(50.f, 50.f),
        std::string{"Test Text!"},
        60,
        testFont.font.get());

    transfer = Transfer{device};
    std::vector<std::shared_ptr<vka::Buffer>> stagingBuffers;

    if (auto cmd = transfer.cmd.lock()) {
      cmd->begin();

      cmd->end();
      device->queueSubmit({}, {cmd}, {}, transfer.fence.get());
    }
    transfer.fence->wait();
    stagingBuffers.clear();
    transfer.fence->reset();

    shapesAsset = loadCollection("content/models/shapes.gltf");
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *shapesAsset.buffer, "ShapesVertexBuffer");
    terrainAsset = loadCollection("content/models/terrain.gltf");
    device->debugNameObject<VkBuffer>(
        VK_OBJECT_TYPE_BUFFER, *terrainAsset.buffer, "TerrainVertexBuffer");

    for (auto& state : bufState) {
    }

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

    engine->run();
  }
};

int main() {
  AppState appState{};

  return 0;
}