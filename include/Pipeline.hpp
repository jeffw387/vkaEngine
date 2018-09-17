#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;
class PipelineLayout;

struct ShaderStageData {
  std::vector<VkSpecializationMapEntry> mapEntries;
  VkSpecializationInfo vkSpecInfo;
  VkPipelineShaderStageCreateInfo createInfo;
};

class GraphicsPipelineCreateInfo {
public:
  GraphicsPipelineCreateInfo(
      VkPipelineLayout layout,
      VkRenderPass renderPass,
      uint32_t subpass);

  void addShaderStage(
      VkShaderStageFlagBits stage,
      std::vector<VkSpecializationMapEntry> mapEntries,
      size_t dataSize,
      void* specializationData,
      VkShaderModule shaderModule,
      const char* entryPoint);

  void addVertexBinding(
      uint32_t binding,
      uint32_t stride,
      VkVertexInputRate inputRate);

  void addVertexAttribute(
      uint32_t location,
      uint32_t binding,
      VkFormat format,
      uint32_t offset);

  void setPrimitiveRestartEnable(VkBool32 primitiveRestartEnable);

  void setTesselationControlPoints(uint32_t patchControlPoints);

  void setPrimitiveTopology(VkPrimitiveTopology topology);

  void addViewportScissor(VkViewport viewport, VkRect2D scissor);

  void setDepthClampEnable(VkBool32 depthClampEnable);

  void setRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable);

  void setPolygonMode(VkPolygonMode polygonMode);

  void setCullMode(VkCullModeFlags cullMode);

  void setFrontFace(VkFrontFace frontFace);

  void setDepthBiasEnable(VkBool32 depthBiasEnable);

  void setDepthBiasFactors(
      float constantFactor,
      float clampFactor,
      float slopeFactor);

  void setLineWidth(float lineWidth);

  void setMultisampleCount(VkSampleCountFlagBits rasterizationSamples);

  void setSampleShadingEnable(VkBool32 sampleShadingEnable);

  void setMinSampleShading(float minSampleShading);

  void setSampleMask(const VkSampleMask* sampleMask);

  void setAlphaToCoverageEnable(VkBool32 alphaToCoverageEnable);

  void setAlphaToOneEnable(VkBool32 alphaToOneEnable);

  void setDepthTestEnable(VkBool32 depthTestEnable);

  void setDepthWriteEnable(VkBool32 depthWriteEnable);

  void setDepthCompareOp(VkCompareOp depthCompareOp);

  void setDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable);

  void setDepthBounds(float minDepthBounds, float maxDepthBounds);

  void addColorBlendAttachment(
      VkBool32 blendEnable,
      VkBlendFactor srcColorBlendFactor,
      VkBlendFactor dstColorBlendFactor,
      VkBlendOp colorBlendOp,
      VkBlendFactor srcAlphaBlendFactor,
      VkBlendFactor dstAlphaBlendFactor,
      VkBlendOp alphaBlendOp,
      VkColorComponentFlags colorWriteMask);

  void setBlendConstants(float r, float g, float b, float a);

  void addDynamicState(VkDynamicState dynamicState);

  operator const VkGraphicsPipelineCreateInfo&();

private:
  std::vector<ShaderStageData> shaderStageData;
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  std::vector<VkVertexInputBindingDescription> vertexBindings;
  std::vector<VkVertexInputAttributeDescription> vertexAttributes;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineTessellationStateCreateInfo tesselationInfo;
  std::vector<VkViewport> viewports;
  std::vector<VkRect2D> scissors;
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  std::vector<VkDynamicState> dynamicStates;
  VkPipelineDynamicStateCreateInfo dynamicInfo;
  VkGraphicsPipelineCreateInfo graphicsCreateInfo;
};

class ComputePipelineCreateInfo {
public:
  operator const VkComputePipelineCreateInfo&() { return computeCreateInfo; }

private:
  ShaderStageData stageData;
  VkPipelineShaderStageCreateInfo stage;
  VkComputePipelineCreateInfo computeCreateInfo;
};

class GraphicsPipeline {
public:
  GraphicsPipeline() = delete;
  GraphicsPipeline(
      Device* device,
      VkPipelineCache cache,
      const VkGraphicsPipelineCreateInfo& createInfo);
  GraphicsPipeline(GraphicsPipeline&&) = default;
  GraphicsPipeline& operator=(GraphicsPipeline&&) = default;
  GraphicsPipeline(const GraphicsPipeline&) = delete;
  GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
  ~GraphicsPipeline();

  VkPipeline getHandle() { return pipelineHandle; }

private:
  Device* device;
  VkPipeline pipelineHandle;
};

class ComputePipeline {
public:
  ComputePipeline() = delete;
  ComputePipeline(
      Device* device,
      VkPipelineCache cache,
      const VkComputePipelineCreateInfo& createInfo);
  ComputePipeline(ComputePipeline&&) = default;
  ComputePipeline& operator=(ComputePipeline&&) = default;
  ComputePipeline(const ComputePipeline&) = delete;
  ComputePipeline& operator=(const ComputePipeline&) = delete;
  ~ComputePipeline();

  VkPipeline getHandle() { return pipelineHandle; }

private:
  Device* device;
  VkPipeline pipelineHandle;
};
}  // namespace vka
