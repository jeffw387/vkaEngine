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

class PipelineCache {
public:
  PipelineCache() = delete;
  PipelineCache(VkDevice device, const std::vector<char>& initialData) {
    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.initialDataSize = initialData.size();
    createInfo.pInitialData = initialData.data();
    vkCreatePipelineCache(device, &createInfo, nullptr, &cacheHandle);
  }
  PipelineCache(VkDevice device) {
    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(device, &createInfo, nullptr, &cacheHandle);
  }
  PipelineCache(const PipelineCache&) = delete;
  PipelineCache& operator=(const PipelineCache&) = delete;
  PipelineCache(PipelineCache&&) = default;
  PipelineCache& operator=(PipelineCache&&) = default;
  ~PipelineCache() { vkDestroyPipelineCache(device, cacheHandle, nullptr); }

  operator VkPipelineCache() { return cacheHandle; }

  auto getCacheData() {
    std::vector<char> cacheData;
    size_t dataSize{};
    vkGetPipelineCacheData(device, cacheHandle, &dataSize, nullptr);
    cacheData.resize(dataSize);
    vkGetPipelineCacheData(device, cacheHandle, &dataSize, cacheData.data());
    return cacheData;
  }

private:
  VkDevice device;
  VkPipelineCache cacheHandle;
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
      VkDevice device,
      VkPipelineCache cache,
      const VkGraphicsPipelineCreateInfo& createInfo);
  GraphicsPipeline(GraphicsPipeline&&) = default;
  GraphicsPipeline& operator=(GraphicsPipeline&&) = default;
  GraphicsPipeline(const GraphicsPipeline&) = delete;
  GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
  ~GraphicsPipeline();

  operator VkPipeline() { return pipelineHandle; }

private:
  VkDevice device;
  VkPipeline pipelineHandle;
};

class ComputePipeline {
public:
  ComputePipeline() = delete;
  ComputePipeline(
      VkDevice device,
      VkPipelineCache cache,
      const VkComputePipelineCreateInfo& createInfo);
  ComputePipeline(ComputePipeline&&) = default;
  ComputePipeline& operator=(ComputePipeline&&) = default;
  ComputePipeline(const ComputePipeline&) = delete;
  ComputePipeline& operator=(const ComputePipeline&) = delete;
  ~ComputePipeline();

  operator VkPipeline() { return pipelineHandle; }

private:
  VkDevice device;
  VkPipeline pipelineHandle;
};
}  // namespace vka
