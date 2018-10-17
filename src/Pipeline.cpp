#include "Pipeline.hpp"
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include "Device.hpp"

namespace vka {
PipelineCache::PipelineCache(VkDevice device, std::vector<char> initialData)
    : device(device) {
  VkPipelineCacheCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  createInfo.initialDataSize = initialData.size();
  createInfo.pInitialData = initialData.data();
  vkCreatePipelineCache(device, &createInfo, nullptr, &cacheHandle);
}

PipelineCache::PipelineCache(VkDevice device) : device(device) {
  VkPipelineCacheCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  vkCreatePipelineCache(device, &createInfo, nullptr, &cacheHandle);
}

PipelineCache::~PipelineCache() {
  if (device != VK_NULL_HANDLE && cacheHandle != VK_NULL_HANDLE) {
    vkDestroyPipelineCache(device, cacheHandle, nullptr);
  }
}

auto PipelineCache::getCacheData() {
  std::vector<char> cacheData;
  size_t dataSize{};
  vkGetPipelineCacheData(device, cacheHandle, &dataSize, nullptr);
  cacheData.resize(dataSize);
  vkGetPipelineCacheData(device, cacheHandle, &dataSize, cacheData.data());
  return cacheData;
}

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(
    VkPipelineLayout layout,
    VkRenderPass renderPass,
    uint32_t subpass) {
  graphicsCreateInfo = VkGraphicsPipelineCreateInfo{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  graphicsCreateInfo.layout = layout;
  graphicsCreateInfo.renderPass = renderPass;
  graphicsCreateInfo.subpass = subpass;

  vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  inputAssemblyInfo = VkPipelineInputAssemblyStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  tesselationInfo = VkPipelineTessellationStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};

  viewportInfo = VkPipelineViewportStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

  rasterizationInfo = VkPipelineRasterizationStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationInfo.lineWidth = 1.0f;

  multisampleInfo = VkPipelineMultisampleStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  depthStencilInfo = VkPipelineDepthStencilStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  colorBlendInfo = VkPipelineColorBlendStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendInfo.blendConstants[0] = 1.f;
  colorBlendInfo.blendConstants[1] = 1.f;
  colorBlendInfo.blendConstants[2] = 1.f;
  colorBlendInfo.blendConstants[3] = 1.f;

  dynamicInfo = VkPipelineDynamicStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
}

void GraphicsPipelineCreateInfo::addShaderStage(
    VkShaderStageFlagBits stage,
    std::vector<VkSpecializationMapEntry> mapEntries,
    size_t dataSize,
    void* specializationData,
    VkShaderModule shaderModule,
    const char* entryPoint) {
  ShaderStageData stageData{};
  stageData.mapEntries = mapEntries;
  stageData.vkSpecInfo.pData = specializationData;
  stageData.vkSpecInfo.dataSize = dataSize;
  stageData.createInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageData.createInfo.stage = stage;
  stageData.createInfo.module = shaderModule;
  stageData.createInfo.pName = entryPoint;
  shaderStageData.push_back(stageData);
}

void GraphicsPipelineCreateInfo::addVertexBinding(
    uint32_t binding,
    uint32_t stride,
    VkVertexInputRate inputRate) {
  VkVertexInputBindingDescription vertexBinding{};
  vertexBinding.binding = binding;
  vertexBinding.stride = stride;
  vertexBinding.inputRate = inputRate;
  vertexBindings.push_back(vertexBinding);
}

void GraphicsPipelineCreateInfo::addVertexAttribute(
    uint32_t location,
    uint32_t binding,
    VkFormat format,
    uint32_t offset) {
  VkVertexInputAttributeDescription vertexAttribute{};
  vertexAttribute.location = location;
  vertexAttribute.binding = binding;
  vertexAttribute.format = format;
  vertexAttribute.offset = offset;
  vertexAttributes.push_back(vertexAttribute);
}

void GraphicsPipelineCreateInfo::setPrimitiveRestartEnable(
    VkBool32 primitiveRestartEnable) {
  inputAssemblyInfo.primitiveRestartEnable = primitiveRestartEnable;
}

void GraphicsPipelineCreateInfo::setPrimitiveTopology(
    VkPrimitiveTopology topology) {
  inputAssemblyInfo.topology = topology;
}

void GraphicsPipelineCreateInfo::addViewportScissor(
    VkViewport viewport,
    VkRect2D scissor) {
  viewports.push_back(viewport);
  scissors.push_back(scissor);
}

void GraphicsPipelineCreateInfo::setFrontFace(VkFrontFace frontFace) {
  rasterizationInfo.frontFace = frontFace;
}

void GraphicsPipelineCreateInfo::setPolygonMode(VkPolygonMode polygonMode) {
  rasterizationInfo.polygonMode = polygonMode;
}

void GraphicsPipelineCreateInfo::setCullMode(VkCullModeFlags cullMode) {
  rasterizationInfo.cullMode = cullMode;
}

void GraphicsPipelineCreateInfo::setLineWidth(float lineWidth) {
  rasterizationInfo.lineWidth = lineWidth;
}

void GraphicsPipelineCreateInfo::setDepthTestEnable(VkBool32 depthTestEnable) {
  depthStencilInfo.depthTestEnable = depthTestEnable;
}

void GraphicsPipelineCreateInfo::setDepthCompareOp(VkCompareOp depthCompareOp) {
  depthStencilInfo.depthCompareOp = depthCompareOp;
}

void GraphicsPipelineCreateInfo::setDepthBoundsTestEnable(
    VkBool32 depthBoundsTestEnable) {
  depthStencilInfo.depthBoundsTestEnable = depthBoundsTestEnable;
}

void GraphicsPipelineCreateInfo::setDepthBounds(
    float minDepthBounds,
    float maxDepthBounds) {
  depthStencilInfo.minDepthBounds = minDepthBounds;
  depthStencilInfo.maxDepthBounds = maxDepthBounds;
}

void GraphicsPipelineCreateInfo::addColorBlendAttachment(
    VkBool32 blendEnable,
    VkBlendFactor srcColorBlendFactor,
    VkBlendFactor dstColorBlendFactor,
    VkBlendOp colorBlendOp,
    VkBlendFactor srcAlphaBlendFactor,
    VkBlendFactor dstAlphaBlendFactor,
    VkBlendOp alphaBlendOp,
    VkColorComponentFlags colorWriteMask) {
  VkPipelineColorBlendAttachmentState blendAttachment{};
  blendAttachment.blendEnable = blendEnable;
  blendAttachment.srcColorBlendFactor = srcColorBlendFactor;
  blendAttachment.dstColorBlendFactor = dstColorBlendFactor;
  blendAttachment.colorBlendOp = colorBlendOp;
  blendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
  blendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
  blendAttachment.alphaBlendOp = alphaBlendOp;
  blendAttachment.colorWriteMask = colorWriteMask;
  colorBlendAttachments.push_back(blendAttachment);
}

void GraphicsPipelineCreateInfo::addDynamicState(VkDynamicState dynamicState) {
  dynamicStates.push_back(dynamicState);
}

GraphicsPipelineCreateInfo::operator const VkGraphicsPipelineCreateInfo&() {
  for (auto& stageData : shaderStageData) {
    stageData.vkSpecInfo.mapEntryCount =
        static_cast<uint32_t>(stageData.mapEntries.size());
    stageData.vkSpecInfo.pMapEntries = stageData.mapEntries.data();
    stageData.createInfo.pSpecializationInfo = &stageData.vkSpecInfo;
    shaderStages.push_back(stageData.createInfo);
  }

  vertexInputInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(vertexBindings.size());
  vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertexAttributes.size());
  vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

  viewportInfo.viewportCount = static_cast<uint32_t>(viewports.size());
  viewportInfo.pViewports = viewports.data();
  viewportInfo.scissorCount = static_cast<uint32_t>(scissors.size());
  viewportInfo.pScissors = scissors.data();

  colorBlendInfo.attachmentCount =
      static_cast<uint32_t>(colorBlendAttachments.size());
  colorBlendInfo.pAttachments = colorBlendAttachments.data();

  dynamicInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicInfo.pDynamicStates = dynamicStates.data();

  graphicsCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  graphicsCreateInfo.pStages = shaderStages.data();
  graphicsCreateInfo.pVertexInputState = &vertexInputInfo;
  graphicsCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
  graphicsCreateInfo.pTessellationState = &tesselationInfo;
  graphicsCreateInfo.pViewportState = &viewportInfo;
  graphicsCreateInfo.pRasterizationState = &rasterizationInfo;
  graphicsCreateInfo.pMultisampleState = &multisampleInfo;
  graphicsCreateInfo.pDepthStencilState = &depthStencilInfo;
  graphicsCreateInfo.pColorBlendState = &colorBlendInfo;
  graphicsCreateInfo.pDynamicState = &dynamicInfo;

  return graphicsCreateInfo;
}

GraphicsPipeline::GraphicsPipeline(
    VkDevice device,
    VkPipelineCache cache,
    const VkGraphicsPipelineCreateInfo& createInfo)
    : device(device) {
  vkCreateGraphicsPipelines(
      device, cache, 1, &createInfo, nullptr, &pipelineHandle);
}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) {
  if (this != &other) {
    device = other.device;
    pipelineHandle = other.pipelineHandle;
    other.device = {};
    other.pipelineHandle = {};
  }
  return *this;
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) {
  *this = std::move(other);
}

GraphicsPipeline::~GraphicsPipeline() {
  if (device != VK_NULL_HANDLE && pipelineHandle != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipelineHandle, nullptr);
  }
}

ComputePipeline::ComputePipeline(
    VkDevice device,
    VkPipelineCache cache,
    const VkComputePipelineCreateInfo& createInfo) {
  vkCreateComputePipelines(
      device, cache, 1, &createInfo, nullptr, &pipelineHandle);
}

ComputePipeline::~ComputePipeline() {
  if (device != VK_NULL_HANDLE && pipelineHandle != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipelineHandle, nullptr);
  }
}

}  // namespace vka