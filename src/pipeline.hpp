#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <memory>
#include <vector>
#include <string_view>
#include <algorithm>
#include <variant>
#include "variant_helper.hpp"
#include <make_fragment_shader.hpp>
#include <make_vertex_shader.hpp>
#include "shader_module.hpp"

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

inline auto make_blend_attachment(blend_attachment blendVariant) {
  return std::visit([](auto blend) { return blend.state; }, blendVariant);
}

template <typename T>
struct shader_stage_state {
  VkPipelineShaderStageCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  VkSpecializationInfo specInfo{};
  VkShaderStageFlagBits shaderStage = {};
  T shaderData = {};
  std::string_view entryPoint = {};
  std::vector<VkSpecializationMapEntry> mapEntries = {};
  const void* pData = {};
  size_t dataSize = {};
};

struct blend_state {
  std::vector<VkPipelineColorBlendAttachmentState> attachments;
  VkPipelineColorBlendStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
};

struct blend_constants {
  float r{1.f};
  float g{1.f};
  float b{1.f};
  float a{1.f};
};

inline auto make_blend_state(
    std::vector<VkPipelineColorBlendAttachmentState> attachments = {},
    blend_constants blendConstants = {}) {
  blend_state blendState;
  blendState.attachments = std::move(attachments);
  blendState.createInfo.blendConstants[0] = blendConstants.r;
  blendState.createInfo.blendConstants[1] = blendConstants.g;
  blendState.createInfo.blendConstants[2] = blendConstants.b;
  blendState.createInfo.blendConstants[3] = blendConstants.a;
  return blendState;
}

struct depth_stencil_state {
  VkPipelineDepthStencilStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
};

inline auto make_depth_stencil_state(
    bool depthTestEnable,
    bool depthWriteEnable) {
  depth_stencil_state state{};
  state.createInfo.depthTestEnable = depthTestEnable;
  state.createInfo.depthWriteEnable = depthWriteEnable;
  return state;
}

struct dynamic_state {
  std::vector<VkDynamicState> states;
  VkPipelineDynamicStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
};

inline auto make_dynamic_state(std::vector<VkDynamicState> dynamicStates = {}) {
  dynamic_state state{};
  state.states = std::move(dynamicStates);
  return state;
}

struct input_assembly_state {
  VkPipelineInputAssemblyStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
};

inline auto make_input_assembly(
    VkPrimitiveTopology topology,
    bool primitiveRestartEnable = false) {
  input_assembly_state state{};
  state.createInfo.topology = topology;
  state.createInfo.primitiveRestartEnable = primitiveRestartEnable;
  return state;
}

struct viewport_state {
  std::vector<VkViewport> viewports;
  std::vector<VkRect2D> scissors;
  VkPipelineViewportStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
};

inline auto make_viewport_state(
    std::vector<VkViewport> viewports = {},
    std::vector<VkRect2D> scissors = {}) {
  viewport_state state{};
  state.viewports = std::move(viewports);
  state.scissors = std::move(scissors);
  return state;
}

struct rasterization_state {
  VkPipelineRasterizationStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
};

inline auto make_rasterization_state(
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE,
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL) {
  rasterization_state state{};
  state.createInfo.cullMode = cullMode;
  state.createInfo.frontFace = frontFace;
  state.createInfo.polygonMode = polygonMode;
  return state;
}

struct multisample_state {
  VkPipelineMultisampleStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
};

inline auto make_multisample_state(
    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_16_BIT) {
  multisample_state state{};
  state.createInfo.rasterizationSamples = rasterizationSamples;
}

struct vertex_state {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;

template <typename T>
inline auto enlarge(std::vector<T>& v, size_t n) {
  if (v.size() < n) {
    v.resize(n);
  }
}

inline auto update_binding = [](VkVertexInputBindingDescription& b,
                                jshd::vertex_input_data input) {
  auto& [binding, stride, inputRate] = b;
  auto offset = input.offset;
  auto extent =
      static_cast<uint32_t>(std::visit(variant_size, input.inputType));
  auto offsetPlusExtent = offset + extent;
  binding = input.binding;
  stride = std::max(stride, offsetPlusExtent);
};

inline auto make_vertex_state(jshd::vertex_shader_data vertexShaderData) {
  vertex_state state{};
  for (jshd::vertex_input_data input : vertexShaderData.inputs) {
    state.attributes.push_back({});
  }
}

struct graphics_pipeline_create_info {
  blend_state blendState;
  depth_stencil_state depthStencilState;
  dynamic_state dynamicState;
  input_assembly_state inputAssemblyState;
  viewport_state viewportState;
  rasterization_state rasterizationState;
  shader_stage_state<jshd::vertex_shader_data> vertexShader;
  shader_stage_state<jshd::fragment_shader_data> fragmentShader;
  VkGraphicsPipelineCreateInfo createInfo{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
};

inline void validate_blend_state(blend_state& blendState) {
  blendState.createInfo.attachmentCount =
      static_cast<uint32_t>(blendState.attachments.size());
  blendState.createInfo.pAttachments = blendState.attachments.data();
}

inline void validate_dynamic_state(dynamic_state& dynamicState) {
  dynamicState.createInfo.dynamicStateCount =
      static_cast<uint32_t>(dynamicState.states.size());
  dynamicState.createInfo.pDynamicStates = dynamicState.states.data();
}

inline void validate_viewport_state(viewport_state& viewportState) {
  viewportState.createInfo.viewportCount =
      static_cast<uint32_t>(viewportState.viewports.size());
  viewportState.createInfo.pViewports = viewportState.viewports.data();
  viewportState.createInfo.scissorCount =
      static_cast<uint32_t>(viewportState.scissors.size());
  viewportState.createInfo.pScissors = viewportState.scissors.data();
}

template <typename T>
inline void validate_shader_stage(shader_stage_state<T>& shaderStageState) {
  auto& [createInfo, specInfo, stage, shaderData, entryPoint, mapEntries, pData,
         dataSize] = shaderStageState;
  specInfo.dataSize = dataSize;
  specInfo.pData = pData;
  specInfo.mapEntryCount = static_cast<uint32_t>(mapEntries.size());
  specInfo.pMapEntries = mapEntries.data();
  createInfo.pSpecializationInfo = &specInfo;
  createInfo.pName = entryPoint.data();
  createInfo.stage = stage;
  createInfo.module = *shaderData.shaderPtr;
}

inline auto make_graphics_pipeline(
    VkDevice device,
    graphics_pipeline_create_info createInfo) {}
}  // namespace vka