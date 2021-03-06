#pragma once
#include <vulkan/vulkan.h>
#include <algorithm>
#include <make_fragment_shader.hpp>
#include <make_vertex_shader.hpp>
#include <memory>
#include <string_view>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <type_traits>
#include <variant>
#include <vector>
#include "shader_module.hpp"
#include "variant_helper.hpp"

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

  operator VkPipeline() const noexcept {
    return m_pipeline;
  }

private:
  VkDevice m_device = {};
  VkPipeline m_pipeline = {};
};

struct no_blend_attachment {
  no_blend_attachment() {
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                           VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT |
                           VK_COLOR_COMPONENT_A_BIT;
  }
  VkPipelineColorBlendAttachmentState state = {};
};

struct alpha_over_attachment {
  alpha_over_attachment() {
    state.blendEnable = true;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                           VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT |
                           VK_COLOR_COMPONENT_A_BIT;
  }
  VkPipelineColorBlendAttachmentState state = {};
};

using blend_attachment = std::
    variant<no_blend_attachment, alpha_over_attachment>;

inline auto make_blend_attachment(
    blend_attachment blendVariant) {
  return std::visit(
      [](auto blend) { return blend.state; }, blendVariant);
}

template <typename T>
struct shader_stage_state {
  VkPipelineShaderStageCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  VkSpecializationInfo specInfo{};
  VkShaderStageFlagBits shaderStage = {};
  shader_data<T>& shaderData;
  std::string_view entryPoint = {};
  std::vector<VkSpecializationMapEntry> mapEntries = {};
  const void* pData = {};
  size_t dataSize = {};

  shader_stage_state(shader_data<T>& shaderData)
      : shaderData(shaderData) {}
};

template <typename T>
inline auto make_shader_stage(
    shader_data<T>& shaderData,
    std::string_view entryPoint,
    gsl::span<char> data) {
  shader_stage_state<T> result{shaderData};
  result.dataSize = data.size();
  result.pData = reinterpret_cast<void*>(data.data());
  uint32_t dataOffset{};
  for (jshd::constant_data& constantData :
       shaderData.shaderData.constants) {
    auto constantSize =
        std::visit(variant_size, constantData.glslType);
    if (constantData.specializationID) {
      VkSpecializationMapEntry mapEntry{};
      mapEntry.constantID = *constantData.specializationID;
      mapEntry.offset = dataOffset;
      mapEntry.size = constantSize;
      result.mapEntries.push_back(mapEntry);
    }
    dataOffset += constantSize;
  }
  if constexpr (std::is_same_v<
                    T,
                    jshd::vertex_shader_data>) {
    result.shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
  } else if constexpr (std::is_same_v<
                           T,
                           jshd::fragment_shader_data>) {
    result.shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  result.entryPoint = entryPoint;
  return result;
}

struct blend_state {
  std::vector<VkPipelineColorBlendAttachmentState>
      attachments;
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
    std::vector<VkPipelineColorBlendAttachmentState>
        attachments = {},
    blend_constants blendConstants = {}) {
  blend_state blendState;
  blendState.attachments = std::move(attachments);
  blendState.createInfo.blendConstants[0] =
      blendConstants.r;
  blendState.createInfo.blendConstants[1] =
      blendConstants.g;
  blendState.createInfo.blendConstants[2] =
      blendConstants.b;
  blendState.createInfo.blendConstants[3] =
      blendConstants.a;
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

inline auto make_dynamic_state(
    std::vector<VkDynamicState> dynamicStates = {}) {
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
  state.createInfo.primitiveRestartEnable =
      primitiveRestartEnable;
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
  state.createInfo.lineWidth = 1.f;
  return state;
}

struct multisample_state {
  VkPipelineMultisampleStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
};

inline auto make_multisample_state(
    VkSampleCountFlagBits rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT) {
  multisample_state state{};
  state.createInfo.rasterizationSamples =
      rasterizationSamples;
  return state;
}

struct vertex_state {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkPipelineVertexInputStateCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
};

template <typename T>
inline auto enlarge(std::vector<T>& v, size_t n) {
  if (v.size() < n) {
    v.resize(n);
  }
}

inline auto update_binding =
    [](VkVertexInputBindingDescription& b,
       jshd::vertex_input_data input) {
      auto& [binding, stride, inputRate] = b;
      auto offset = input.offset;
      auto extent = static_cast<uint32_t>(
          std::visit(variant_size, input.inputType));
      auto offsetPlusExtent = offset + extent;
      binding = input.binding;
      stride = std::max(stride, offsetPlusExtent);
    };

struct glsl_type_format_visitor {
  VkFormat operator()(glm::float32 f) {
    return VK_FORMAT_R32_SFLOAT;
  }

  VkFormat operator()(glm::vec2 v) {
    return VK_FORMAT_R32G32_SFLOAT;
  }

  VkFormat operator()(glm::vec3 v) {
    return VK_FORMAT_R32G32B32_SFLOAT;
  }

  VkFormat operator()(glm::vec4 v) {
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  }

  template <typename T>
  VkFormat operator()(T defaultType) {
    return VK_FORMAT_UNDEFINED;
  }
};

inline auto set_attribute =
    [](VkVertexInputAttributeDescription& a,
       jshd::vertex_input_data input) {
      auto& [location, binding, format, offset] = a;
      location = input.location;
      binding = input.binding;
      format = std::visit(
          glsl_type_format_visitor{}, input.inputType);
      offset = input.offset;
    };

inline auto make_vertex_state(
    jshd::vertex_shader_data vertexShaderData) {
  vertex_state state{};
  for (jshd::vertex_input_data input :
       vertexShaderData.inputs) {
    enlarge(state.bindings, input.binding + 1);
    update_binding(state.bindings[input.binding], input);
    enlarge(state.attributes, input.location + 1);
    set_attribute(state.attributes[input.location], input);
  }
  return state;
}

auto size32 = [](const auto& container) {
  return static_cast<uint32_t>(container.size());
};

inline void validate_vertex_state(
    vertex_state& vertexState) {
  auto& [sType,
         pNext,
         flags,
         bindingCount,
         pBindings,
         attribCount,
         pAttribs] = vertexState.createInfo;
  bindingCount = size32(vertexState.bindings);
  pBindings = vertexState.bindings.data();
  attribCount = size32(vertexState.attributes);
  pAttribs = vertexState.attributes.data();
}

inline void validate_blend_state(blend_state& blendState) {
  blendState.createInfo.attachmentCount =
      static_cast<uint32_t>(blendState.attachments.size());
  blendState.createInfo.pAttachments =
      blendState.attachments.data();
}

inline void validate_dynamic_state(
    dynamic_state& dynamicState) {
  dynamicState.createInfo.dynamicStateCount =
      static_cast<uint32_t>(dynamicState.states.size());
  dynamicState.createInfo.pDynamicStates =
      dynamicState.states.data();
}

inline void validate_viewport_state(
    viewport_state& viewportState) {
  viewportState.createInfo.viewportCount =
      static_cast<uint32_t>(viewportState.viewports.size());
  viewportState.createInfo.pViewports =
      viewportState.viewports.data();
  viewportState.createInfo.scissorCount =
      static_cast<uint32_t>(viewportState.scissors.size());
  viewportState.createInfo.pScissors =
      viewportState.scissors.data();
}

template <typename T>
inline void validate_shader_stage(
    shader_stage_state<T>& shaderStageState) {
  auto& [createInfo,
         specInfo,
         stage,
         shaderData,
         entryPoint,
         mapEntries,
         pData,
         dataSize] = shaderStageState;
  specInfo.dataSize = dataSize;
  specInfo.pData = pData;
  specInfo.mapEntryCount =
      static_cast<uint32_t>(mapEntries.size());
  specInfo.pMapEntries = mapEntries.data();
  createInfo.pSpecializationInfo = &specInfo;
  createInfo.pName = entryPoint.data();
  createInfo.stage = stage;
  createInfo.module = *shaderData.shaderPtr;
}

inline auto make_pipeline(
    VkDevice device,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkPipelineLayout layout,
    VkPipelineCache cache,
    blend_state blendState,
    depth_stencil_state depthStencilState,
    dynamic_state dynamicState,
    input_assembly_state inputAssemblyState,
    viewport_state viewportState,
    rasterization_state rasterizationState,
    multisample_state multisampleState,
    vertex_state vertexState,
    shader_stage_state<jshd::vertex_shader_data>&
        vertexShader,
    shader_stage_state<jshd::fragment_shader_data>&
        fragmentShader) {
  validate_blend_state(blendState);
  validate_dynamic_state(dynamicState);
  validate_viewport_state(viewportState);
  validate_vertex_state(vertexState);
  validate_shader_stage(vertexShader);
  validate_shader_stage(fragmentShader);

  VkGraphicsPipelineCreateInfo createInfo{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  createInfo.renderPass = renderPass;
  createInfo.subpass = subpass;
  createInfo.layout = layout;
  createInfo.pColorBlendState = &blendState.createInfo;
  createInfo.pDepthStencilState =
      &depthStencilState.createInfo;
  createInfo.pDynamicState = &dynamicState.createInfo;
  createInfo.pInputAssemblyState =
      &inputAssemblyState.createInfo;
  createInfo.pViewportState = &viewportState.createInfo;
  createInfo.pRasterizationState =
      &rasterizationState.createInfo;
  createInfo.pMultisampleState =
      &multisampleState.createInfo;
  createInfo.pVertexInputState = &vertexState.createInfo;
  std::vector<VkPipelineShaderStageCreateInfo> stages = {
      vertexShader.createInfo, fragmentShader.createInfo};
  createInfo.stageCount =
      static_cast<uint32_t>(stages.size());
  createInfo.pStages = stages.data();

  VkPipeline pipelineHandle;
  auto pipelineResult = vkCreateGraphicsPipelines(
      device,
      cache,
      1,
      &createInfo,
      nullptr,
      &pipelineHandle);
  if (pipelineResult != VK_SUCCESS) {
    exit(pipelineResult);
  }
  return std::make_unique<pipeline>(device, pipelineHandle);
}
}  // namespace vka