#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <memory>
#include <vector>
#include <string_view>
#include <any>
#include <variant>

namespace vka {
struct pipeline {
  pipeline(VkDevice device, VkPipeline pipelineHandle)
      : m_device(device), m_pipeline(pipelineHandle) {}

  pipeline(const pipeline&) = delete;
  pipeline(pipeline&&) = default;
  pipeline& operator=(const pipeline&) = delete;
  pipeline& operator=(pipeline&&) = default;

  ~pipeline() noexcept { vkDestroyPipeline(m_device, m_pipeline, nullptr); }
  
  operator VkPipeline() const noexcept { return m_pipeline; }

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

  shader_stage_builder& shader_module(
      VkShaderModule shaderModule,
      std::string_view entryPoint) {
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

struct no_blend_attachment {
  no_blend_attachment() {
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }
  VkPipelineColorBlendAttachmentState state = {};
};

struct alpha_over_attachment {
  alpha_over_attachment() {
    state.blendEnable = true;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }
  VkPipelineColorBlendAttachmentState state = {};
};

using blend_attachment =
    std::variant<no_blend_attachment, alpha_over_attachment>;

struct graphics_pipeline_builder {
  tl::expected<std::unique_ptr<pipeline>, VkResult> build(VkDevice device) {
    m_vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_vertexBindings.size());
    m_vertexInputInfo.pVertexBindingDescriptions = m_vertexBindings.data();
    m_vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_vertexAttributes.size());
    m_vertexInputInfo.pVertexAttributeDescriptions = m_vertexAttributes.data();

    m_viewportCreateInfo.viewportCount =
        static_cast<uint32_t>(m_viewports.size());
    m_viewportCreateInfo.pViewports = m_viewports.data();
    m_viewportCreateInfo.scissorCount =
        static_cast<uint32_t>(m_scissors.size());
    m_viewportCreateInfo.pScissors = m_scissors.data();

    m_rasterizationCreateInfo.lineWidth = 1.f;

    m_blendCreateInfo.attachmentCount =
        static_cast<uint32_t>(m_colorAttachmentBlendStates.size());
    m_blendCreateInfo.pAttachments = m_colorAttachmentBlendStates.data();
    for (int i = {}; i < 4; ++i) {
      m_blendCreateInfo.blendConstants[i] = 1.f;
    }

    m_dynamicStateCreateInfo.dynamicStateCount =
        static_cast<uint32_t>(m_dynamicStates.size());
    m_dynamicStateCreateInfo.pDynamicStates = m_dynamicStates.data();

    m_createInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    m_createInfo.pStages = m_shaderStages.data();
    m_createInfo.pVertexInputState = &m_vertexInputInfo;
    m_createInfo.pInputAssemblyState = &m_inputAssemblyInfo;
    m_createInfo.pTessellationState =
        (m_tesselationInfo ? &*m_tesselationInfo : nullptr);
    m_createInfo.pViewportState = &m_viewportCreateInfo;
    m_createInfo.pRasterizationState = &m_rasterizationCreateInfo;
    m_createInfo.pMultisampleState = &m_multisampleCreateInfo;
    m_createInfo.pDepthStencilState = &m_depthStencilCreateInfo;
    m_createInfo.pColorBlendState = &m_blendCreateInfo;
    m_createInfo.pDynamicState = &m_dynamicStateCreateInfo;

    VkPipeline pipelineHandle = {};
    auto result = vkCreateGraphicsPipelines(
        device, m_cache, 1, &m_createInfo, nullptr, &pipelineHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<pipeline>(device, pipelineHandle);
  }

  graphics_pipeline_builder& pipeline_layout(VkPipelineLayout layout) {
    m_createInfo.layout = layout;
    return *this;
  }

  graphics_pipeline_builder& render_pass(
      VkRenderPass renderPass,
      uint32_t subpass) {
    m_createInfo.renderPass = renderPass;
    m_createInfo.subpass = subpass;
    return *this;
  }

  graphics_pipeline_builder& shader_stage(shader_stage_data stageData) {
    m_specializationMaps.push_back(std::move(stageData.mapEntries));

    VkSpecializationInfo specInfo = {};
    specInfo.dataSize = stageData.dataSize;
    specInfo.pData = stageData.pData;
    specInfo.mapEntryCount =
        static_cast<uint32_t>(m_specializationMaps.back().size());
    specInfo.pMapEntries = m_specializationMaps.back().data();
    m_shaderSpecializations.push_back(std::move(specInfo));

    VkPipelineShaderStageCreateInfo stageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stageCreateInfo.stage = stageData.shaderStage;
    stageCreateInfo.module = stageData.shaderModule;
    stageCreateInfo.pName = stageData.entryPoint.data();
    stageCreateInfo.pSpecializationInfo = &m_shaderSpecializations.back();
    m_shaderStages.push_back(std::move(stageCreateInfo));
    return *this;
  }

  template <typename T>
  graphics_pipeline_builder& vertex_binding(
      uint32_t binding,
      VkVertexInputRate inputRate) {
    m_vertexBindings.push_back(
        {binding, static_cast<uint32_t>(sizeof(T)), inputRate});
    return *this;
  }

  graphics_pipeline_builder& vertex_attribute(
      uint32_t location,
      uint32_t binding,
      uint8_t elementCount,
      uint32_t offset) {
    VkFormat attributeFormat = {};
    switch (elementCount) {
      default:
      case 1:
        attributeFormat = VK_FORMAT_R32_SFLOAT;
        break;
      case 2:
        attributeFormat = VK_FORMAT_R32G32_SFLOAT;
        break;
      case 3:
        attributeFormat = VK_FORMAT_R32G32B32_SFLOAT;
        break;
      case 4:
        attributeFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
    }
    m_vertexAttributes.push_back({location, binding, attributeFormat, offset});
    return *this;
  }

  graphics_pipeline_builder& primitive_topology(VkPrimitiveTopology topology) {
    m_inputAssemblyInfo.topology = topology;
    return *this;
  }

  graphics_pipeline_builder& enable_primitive_restart() {
    m_inputAssemblyInfo.primitiveRestartEnable = true;
    return *this;
  }

  graphics_pipeline_builder& viewport_scissor(
      VkViewport viewport,
      VkRect2D scissor) {
    m_viewports.push_back(std::move(viewport));
    m_scissors.push_back(std::move(scissor));
    return *this;
  }

  graphics_pipeline_builder& polygon_mode(VkPolygonMode polygonMode) {
    m_rasterizationCreateInfo.polygonMode = polygonMode;
    return *this;
  }

  graphics_pipeline_builder& cull_mode(VkCullModeFlags cullMode) {
    m_rasterizationCreateInfo.cullMode = cullMode;
    return *this;
  }

  graphics_pipeline_builder& winding_cw() {
    m_rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    return *this;
  }

  // note: defaults to ccw, so maybe not needed
  // graphics_pipeline_builder& winding_ccw() {
  //   m_rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  //   return *this;
  // }

  graphics_pipeline_builder& depth_test() {
    m_depthStencilCreateInfo.depthTestEnable = true;
    m_depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    return *this;
  }

  graphics_pipeline_builder& depth_write() {
    m_depthStencilCreateInfo.depthWriteEnable = true;
    return *this;
  }

  graphics_pipeline_builder& color_attachment(
      blend_attachment blendAttachment) {
    m_colorAttachmentBlendStates.push_back(std::visit(
        [&](auto attachmentVariant) { return attachmentVariant.state; },
        blendAttachment));
    return *this;
  }

  graphics_pipeline_builder& dynamic_state(VkDynamicState state) {
    m_dynamicStates.push_back(state);
    return *this;
  }

private:
  std::vector<std::vector<VkSpecializationMapEntry>> m_specializationMaps = {};
  std::vector<VkSpecializationInfo> m_shaderSpecializations = {};
  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages = {};

  std::vector<VkVertexInputBindingDescription> m_vertexBindings = {};
  std::vector<VkVertexInputAttributeDescription> m_vertexAttributes = {};
  VkPipelineVertexInputStateCreateInfo m_vertexInputInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

  tl::optional<VkPipelineTessellationStateCreateInfo> m_tesselationInfo = {};

  std::vector<VkViewport> m_viewports = {};
  std::vector<VkRect2D> m_scissors = {};
  VkPipelineViewportStateCreateInfo m_viewportCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

  VkPipelineRasterizationStateCreateInfo m_rasterizationCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

  VkPipelineMultisampleStateCreateInfo m_multisampleCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_SAMPLE_COUNT_1_BIT};

  VkPipelineDepthStencilStateCreateInfo m_depthStencilCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  std::vector<VkPipelineColorBlendAttachmentState>
      m_colorAttachmentBlendStates = {};
  VkPipelineColorBlendStateCreateInfo m_blendCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

  std::vector<VkDynamicState> m_dynamicStates = {};
  VkPipelineDynamicStateCreateInfo m_dynamicStateCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

  VkGraphicsPipelineCreateInfo m_createInfo = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  VkPipelineCache m_cache = {};
};
}  // namespace vka