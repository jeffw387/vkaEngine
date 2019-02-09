#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <memory>
#include <vector>
#include <string_view>
#include <any>
#include <variant>
#include <make_shader.hpp>
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

struct shader_stage_data {
  VkShaderStageFlagBits shaderStage = {};
  VkShaderModule shaderModule = {};
  std::string_view entryPoint = {};
  std::vector<VkSpecializationMapEntry> mapEntries = {};
  const void* pData = {};
  size_t dataSize = {};
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
    std::vector<VkPipelineColorBlendAttachmentState> attachments,
    blend_constants blendConstants = {}) {
  blend_state blendState;
  blendState.attachments = std::move(attachments);
  blendState.createInfo.blendConstants[0] = blendConstants.r;
  blendState.createInfo.blendConstants[1] = blendConstants.g;
  blendState.createInfo.blendConstants[2] = blendConstants.b;
  blendState.createInfo.blendConstants[3] = blendConstants.a;
  return blendState;
}

struct graphics_pipeline_create_info {
  blend_state blendState;
};
}
}  // namespace vka