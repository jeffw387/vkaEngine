#pragma once
#include <memory>
#include <vector>
#include <experimental/filesystem>

#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "ShaderModule.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"

namespace fs = std::experimental::filesystem;

namespace p3d {
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

struct FragmentSpecData {
  uint32_t materialCount;
  uint32_t lightCount;
};

struct FragmentPushConstants {
  uint32_t materialIndex;
};

struct BufferedData {

};

struct Pipeline {
  std::unique_ptr<vka::ShaderModule> vertexShader;
  std::unique_ptr<vka::ShaderModule> fragmentShader;
  VkDescriptorSetLayoutBinding materialBinding;
  VkDescriptorSetLayoutBinding dynamicLightBinding;
  VkDescriptorSetLayoutBinding lightDataBinding;
  VkDescriptorSetLayoutBinding cameraBinding;
  VkDescriptorSetLayoutBinding instanceBinding;
  std::unique_ptr<vka::DescriptorSetLayout> materialLayout;
  std::unique_ptr<vka::DescriptorSetLayout> dynamicLightLayout;
  std::unique_ptr<vka::DescriptorSetLayout> lightDataLayout;
  std::unique_ptr<vka::DescriptorSetLayout> cameraLayout;
  std::unique_ptr<vka::DescriptorSetLayout> instanceLayout;
  std::shared_ptr<vka::PipelineLayout> pipelineLayout;
  std::shared_ptr<vka::GraphicsPipeline> pipeline;

  Pipeline() {}
  Pipeline(
      vka::Device* device,
      vka::RenderPass* renderPass,
      vka::PipelineCache* pipelineCache)
      : vertexShader(
            device->createShaderModule("content/shaders/shader.vert.spv")),
        fragmentShader(
            device->createShaderModule("content/shaders/shader.frag.spv")),
        materialBinding({0,
                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                         1,
                         VK_SHADER_STAGE_FRAGMENT_BIT,
                         nullptr}),
        dynamicLightBinding({0,
                             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             1,
                             VK_SHADER_STAGE_FRAGMENT_BIT,
                             nullptr}),
        lightDataBinding({0,
                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                          1,
                          VK_SHADER_STAGE_FRAGMENT_BIT,
                          nullptr}),
        cameraBinding({0,
                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       1,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       nullptr}),
        instanceBinding({0,
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                         1,
                         VK_SHADER_STAGE_VERTEX_BIT,
                         nullptr}),
        materialLayout(device->createSetLayout({materialBinding})),
        dynamicLightLayout(device->createSetLayout({dynamicLightBinding})),
        lightDataLayout(device->createSetLayout({lightDataBinding})),
        cameraLayout(device->createSetLayout({cameraBinding})),
        instanceLayout(device->createSetLayout({instanceBinding})),
        pipelineLayout(device->createPipelineLayout(
            {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}},
            {*materialLayout,
             *dynamicLightLayout,
             *lightDataLayout,
             *cameraLayout,
             *instanceLayout})) {
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
    pipeline3DInfo.setFrontFace(VK_FRONT_FACE_CLOCKWISE);

    pipeline = device->createGraphicsPipeline(*pipelineCache, pipeline3DInfo);
  }
};
}