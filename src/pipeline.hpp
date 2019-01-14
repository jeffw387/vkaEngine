#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <memory>
#include <vector>
#include <string_view>
#include <any>

namespace vka {
struct pipeline {
  pipeline(VkDevice device, VkPipeline pipelineHandle) 
    : m_device(device), m_pipeline(pipelineHandle) {}
  pipeline(const pipeline&) = delete;
  pipeline(pipeline&&) = default;
  pipeline& operator=(const pipeline&) = delete;
  pipeline& operator=(pipeline&&) = default;
  ~pipeline() noexcept {
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
  }
private:
  VkDevice m_device = {};
  VkPipeline m_pipeline = {};
};

struct shader_stage_data {
  VkShaderStageFlagBits shaderStage = {};
  VkShaderModule shaderModule = {};
  std::string_view entryPoint = {};
  std::vector<VkSpecializationMapEntry> mapEntries = {};
  const void* pData = {};
  size_t dataSize = {};
};

struct shader_stage_builder {
  shader_stage_data build() {
    m_stageData.dataSize = m_currentSize;
    return m_stageData;
  }

  shader_stage_builder& shader_module(VkShaderModule shaderModule, std::string_view entryPoint) {
    m_stageData.shaderModule = shaderModule;
    m_stageData.entryPoint = entryPoint;
    return *this;
  }

  template <typename T>
  shader_stage_builder& add_constant(uint32_t id) {
    VkSpecializationMapEntry entry = {};
    entry.constantID = id;
    entry.offset = m_currentSize;
    entry.size = sizeof(T);
    m_currentSize += sizeof(T);
    m_stageData.mapEntries.push_back(std::move(entry));
    return *this;
  }

  shader_stage_builder& constant_data(const void* pData) {
    m_stageData.pData = pData;
    return *this;
  }

  shader_stage_builder& vertex() {
    m_stageData.shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
    return *this;
  }

  shader_stage_builder& fragment() {
    m_stageData.shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    return *this;
  }

private:
  size_t m_currentSize = {};
  shader_stage_data m_stageData = {};
};

struct graphics_pipeline_builder {
  tl::expected<std::unique_ptr<pipeline>, VkResult> build(VkDevice device) {
    m_createInfo.;
    
    VkPipeline pipelineHandle = {};
    auto result = vkCreateGraphicsPipelines(device, m_cache, 1, &m_createInfo, nullptr, &pipelineHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<pipeline>(device, pipelineHandle);
  }

  graphics_pipeline_builder& pipeline_layout(VkPipelineLayout layout) {
    m_createInfo.layout = layout;
    return *this;
  }

  graphics_pipeline_builder& shader_stage(shader_stage_data stageData) {
    m_specializationMaps.push_back(std::move(stageData.mapEntries));
    
    VkSpecializationInfo specInfo = {};
    specInfo.dataSize = stageData.dataSize;
    specInfo.pData = stageData.pData;
    specInfo.mapEntryCount = static_cast<uint32_t>(m_specializationMaps.back().size());
    specInfo.pMapEntries = m_specializationMaps.back().data();
    m_shaderSpecializations.push_back(std::move(specInfo));
    
    VkPipelineShaderStageCreateInfo stageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stageCreateInfo.stage = stageData.shaderStage;
    stageCreateInfo.module = stageData.shaderModule;
    stageCreateInfo.pName = stageData.entryPoint.data();
    stageCreateInfo.pSpecializationInfo = &m_shaderSpecializations.back();
    m_shaderStages.push_back(std::move(stageCreateInfo));
  }

  template <typename T>
  graphics_pipeline_builder& vertex_binding(uint32_t binding, VkVertexInputRate inputRate) {
    m_vertexBindings.push_back()
  }
private:
  std::vector<std::vector<VkSpecializationMapEntry>> m_specializationMaps = {};
  std::vector<VkSpecializationInfo> m_shaderSpecializations = {};
  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages = {};

  std::vector<VkVertexInputBindingDescription> m_vertexBindings = {};
  std::vector<VkVertexInputAttributeDescription> m_vertexAttributes = {};
  VkPipelineVertexInputStateCreateInfo m_vertexInputInfo = {};

  VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo = {};

  tl::optional<VkPipelineTessellationStateCreateInfo> m_tesselationInfo = {};

  std::vector<VkViewport> m_viewports = {};
  std::vector<VkRect2D> m_scissors = {};
  VkPipelineViewportStateCreateInfo m_viewportCreateInfo = {};

  VkPipelineRasterizationStateCreateInfo m_rasterizationCreateInfo = {};

  VkPipelineMultisampleStateCreateInfo m_multisampleCreateInfo = {};

  VkPipelineDepthStencilStateCreateInfo m_depthStencilCreateInfo = {};

  std::vector<VkPipelineColorBlendAttachmentState> m_colorAttachmentBlendStates = {};
  VkPipelineColorBlendStateCreateInfo m_blendCreateInfo = {};

  std::vector<VkDynamicState> m_dynamicStates = {};
  VkPipelineDynamicStateCreateInfo m_dynamicStateCreateInfo = {};

  VkGraphicsPipelineCreateInfo m_createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  VkPipelineCache m_cache = {};
};
}