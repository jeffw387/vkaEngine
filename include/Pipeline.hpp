#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;
class PipelineLayout;

class Pipeline {
public:
  Pipeline() = delete;
  Pipeline(Device* device, VkPipelineLayout layout);
  Pipeline(Pipeline&&) = default;
  Pipeline& operator=(Pipeline&&) = default;
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;
  ~Pipeline();

  void compile();
  VkPipeline getHandle() { return pipelineHandle; }
private:
  Device* device;
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineTessellationStateCreateInfo tesselationInfo;
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDynamicStateCreateInfo dynamicInfo;
  VkPipelineLayout layout;
  VkRenderPass renderPass;
  uint32_t subpass;
  VkComputePipelineCreateInfo computeCreateInfo;
  VkGraphicsPipelineCreateInfo graphicsCreateInfo;
  VkPipeline pipelineHandle;
};
}  // namespace vka
