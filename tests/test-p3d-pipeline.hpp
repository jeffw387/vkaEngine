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

struct P3DPipeline {
  std::unique_ptr<vka::ShaderModule> vertexShader;
  std::unique_ptr<vka::ShaderModule> fragmentShader;
  VkDescriptorSetLayoutBinding materialBinding;
  VkDescriptorSetLayoutBinding dynamicLightBinding;
  VkDescriptorSetLayoutBinding lightDataBinding;
  VkDescriptorSetLayoutBinding cameraBinding;
  VkDescriptorSetLayoutBinding instanceBinding;
  std::vector<std::unique_ptr<vka::DescriptorSetLayout>> descriptorSetLayouts;
  std::shared_ptr<vka::PipelineLayout> pipelineLayout;
  std::shared_ptr<vka::GraphicsPipeline> pipeline;

  P3DPipeline() {}
  P3DPipeline(
    vka::Device* device,
    fs::path vertexShaderPath,
    fs::path fragmentShaderPath),
    vka::PipelineCache pipelineCache :
    vertexShader(device->createShaderModule("content/shaders/shader.vert.spv")),
    fragmentShader(device->createShaderModule("content/shaders/shader.frag.spv")),
    materialBinding({
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr}),
    dynamicLightsBinding({
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr}),
    lightDataBinding({
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr}),
    cameraBinding({
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr}),
    instanceBinding({
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr}),
    descriptorSetLayouts({
      device->createSetLayout({materialBinding})
      device->createSetLayout({dynamicLightsBinding})
      device->createSetLayout({lightDataBinding})
      device->createSetLayout({cameraBinding})
      device->createSetLayout({instanceBinding})
    }),
    pipelineLayout({{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}},
        {*descriptorSetLayouts[0],
         *descriptorSetLayouts[1],
         *descriptorSetLayouts[2],
         *descriptorSetLayouts[3],
         *descriptorSetLayouts[4]}) {
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