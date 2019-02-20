#pragma once

#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <make_fragment_shader.hpp>
#include <make_vertex_shader.hpp>
#include "descriptor_set_layout.hpp"
#include "shader_module.hpp"

namespace vka {
struct pipeline_layout {
  pipeline_layout(VkDevice device, VkPipelineLayout layout)
      : m_device(device), m_layout(layout) {}
  pipeline_layout(const pipeline_layout&) = delete;
  pipeline_layout(pipeline_layout&&) = default;
  pipeline_layout& operator=(const pipeline_layout&) = delete;
  pipeline_layout& operator=(pipeline_layout&&) = default;
  ~pipeline_layout() noexcept {
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
  }
  operator VkPipelineLayout() const noexcept { return m_layout; }

private:
  VkDevice m_device = {};
  VkPipelineLayout m_layout = {};
};



inline auto make_pipeline_layout(
    VkDevice device,
    shader_data<jshd::vertex_shader_data>& vertexShaderData,
    shader_data<jshd::fragment_shader_data>& fragmentShaderData,
    std::vector<set_data> setData) {
  std::vector<VkDescriptorSetLayout> layouts;
  layouts.reserve(setData.size());
  std::vector<VkPushConstantRange> pushRanges;

  for (const set_data& set : setData) {
    layouts.push_back(*set.setLayoutPtr);
  }

  auto getPushRanges = [&pushRanges](auto& shaderModuleData) {
    auto& [ptr, shaderData] = shaderModuleData;
    auto shaderStage = get_shader_stage<decltype(shaderData)>();
    for (auto& push : shaderData.pushConstants) {
      // TODO: need range from shader for each push constant
      pushRanges.push_back(
          {VkShaderStageFlags{} | shaderStage,
           push.offset,
           std::visit(
               [](auto v) { return static_cast<uint32_t>(sizeof(v)); },
               push.glslType)});
    }
  };
  getPushRanges(vertexShaderData);
  getPushRanges(fragmentShaderData);

  VkPipelineLayoutCreateInfo createInfo{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  createInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
  createInfo.pSetLayouts = layouts.data();
  createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
  createInfo.pPushConstantRanges = pushRanges.data();
  VkPipelineLayout pipelineLayout{};
  auto createResult =
      vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout);
  if (createResult != VK_SUCCESS) {
    exit(createResult);
  }
  return std::make_unique<pipeline_layout>(device, pipelineLayout);
}
}  // namespace vka