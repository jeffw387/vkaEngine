#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <experimental/filesystem>

#include "Text.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "ShaderModule.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "Device.hpp"
#include "CommandPool.hpp"
#include "test-transfer.hpp"

namespace fs = std::experimental::filesystem;

struct TextObject {
  glm::vec2 screenPosition;
  std::string str;
  int pixelHeight;
  Text::Font<>* font;
  glm::vec4 color;
  TextObject(
      glm::vec2 screenPosition,
      std::string str,
      int pixelHeight,
      Text::Font<>* font,
      glm::vec4 color = glm::vec4(0.f))
      : screenPosition(std::move(screenPosition)),
        str(std::move(str)),
        pixelHeight(pixelHeight),
        font(font),
        color(color) {}
};

struct TextPushData {
  struct TextVertexPushData {
    glm::vec2 screenPos;       // per draw        - vert
    glm::vec2 clipSpaceScale;  // per frame       - vert/frag
    glm::float32 textScale;    // per text object - vert
    glm::vec3 padding;
  } vertex;
  struct TextFragmentPushData {
    glm::vec4 fontColor;          // per text object - frag
    glm::float32 distanceFactor;  // per frame       - vert/frag
    // TODO: send glyph index as third part of uv coords?
    glm::uint32 glyphIndex;  // per draw        - frag
    glm::float32 padding[2];
  } fragment;
};

struct TextPipeline {
  std::unique_ptr<vka::ShaderModule> vertexShader;
  std::unique_ptr<vka::ShaderModule> fragmentShader;
  std::unique_ptr<vka::Sampler> sampler;
  std::unique_ptr<vka::DescriptorSetLayout> setLayout;
  std::unique_ptr<vka::DescriptorPool> descriptorPool;
  std::shared_ptr<vka::PipelineLayout> pipelineLayout;
  std::shared_ptr<vka::GraphicsPipeline> pipeline;

  TextPipeline() {}
  TextPipeline(
      vka::Device* device,
      vka::RenderPass* renderPass,
      vka::PipelineCache* pipelineCache)
      : vertexShader(
            device->createShaderModule("content/shaders/text.vert.spv")),
        fragmentShader(
            device->createShaderModule("content/shaders/text.frag.spv")),
        sampler(device->createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR)),
        setLayout(
            device->createSetLayout({{0,
                                      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                      1,
                                      VK_SHADER_STAGE_FRAGMENT_BIT,
                                      *sampler}})),
        descriptorPool(device->createDescriptorPool(
            {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
            1)),
        pipelineLayout(device->createPipelineLayout(
            {{VK_SHADER_STAGE_VERTEX_BIT,
              offsetof(TextPushData, vertex),
              sizeof(TextPushData::TextVertexPushData)},
             {VK_SHADER_STAGE_FRAGMENT_BIT,
              offsetof(TextPushData, fragment),
              sizeof(TextPushData::TextFragmentPushData)}},
            {*setLayout})) {
    vka::GraphicsPipelineCreateInfo textPipelineInfo{
        *pipelineLayout, *renderPass, 1};
    textPipelineInfo.addColorBlendAttachment(
        true,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    textPipelineInfo.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    textPipelineInfo.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    textPipelineInfo.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT, {}, 0, nullptr, *vertexShader, "main");
    textPipelineInfo.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT, {}, 0, nullptr, *fragmentShader, "main");
    textPipelineInfo.addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
    textPipelineInfo.addVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, 8);
    textPipelineInfo.addVertexBinding(
        0, sizeof(Text::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    textPipelineInfo.addViewportScissor({}, {});
    textPipelineInfo.setCullMode(VK_CULL_MODE_NONE);

    pipeline = device->createGraphicsPipeline(*pipelineCache, textPipelineInfo);
  }
};

struct Font {
  std::unique_ptr<Text::Font<>> font;
  std::unique_ptr<Text::VertexData> vertexData;
  std::shared_ptr<vka::Buffer> indexBuffer;
  std::shared_ptr<vka::Buffer> vertexBuffer;
  uint32_t textureSize = {};
  uint32_t layerCount = {};
  std::vector<uint8_t> pixels;
  std::shared_ptr<vka::Image> image;
  std::shared_ptr<vka::ImageView> imageView;
  std::shared_ptr<vka::DescriptorSet> descriptorSet;

  Font() {}
  Font(fs::path fontPath, vka::Device* device, TextPipeline& pipeline)
      : font(std::make_unique<Text::Font<>>(fontPath)),
        vertexData(font->getVertexData()),
        indexBuffer(device->createBuffer(
            vertexData->indices.size() * sizeof(Text::Index),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            true)),
        vertexBuffer(device->createBuffer(
            vertexData->vertices.size() * sizeof(Text::Vertex),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            true)),
        textureSize(static_cast<uint32_t>(font->getTextureSize())),
        layerCount(static_cast<uint32_t>(font->getTextureLayerCount())),
        pixels(font->getTextureData()),
        image(device->createImageArray2D(
            {textureSize, textureSize},
            layerCount,
            VK_FORMAT_R8G8B8A8_UINT,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            vka::ImageAspect::Color,
            true)),
        imageView(
            device->createImageView2D(image, image->format, image->aspect)),
        descriptorSet(pipeline.descriptorPool->allocateDescriptorSet(
            pipeline.setLayout.get())) {
    auto descriptor = descriptorSet->getDescriptor<vka::ImageSamplerDescriptor>(
        vka::DescriptorReference{});
    (*descriptor)(*imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    descriptorSet->validate(*device);
  }
};