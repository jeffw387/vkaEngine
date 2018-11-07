#include "CommandPool.hpp"
#include "Device.hpp"

namespace vka {

void CommandBuffer::checkInitial() {
if (state != Initial) {
    MultiLogger::get()->error("Incorrect cmd buffer state {}. Should be in initial state.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRecording() {
  if (state != Recording) {
    MultiLogger::get()->error("Incorrect cmd buffer state {}. Should be in recording state.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkExecutable() {
  if (state != Executable) {
    MultiLogger::get()->error("Incorrect cmd buffer state {}. Should be in executable state.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkPending() {
  if (state != Pending) {
    MultiLogger::get()->error("Incorrect cmd buffer state {}. Should be in pending state.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRenderPassActive() {
  if (!activeRenderPass) {
    MultiLogger::get()->error("Cannot record this command outside of a render pass.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRenderPassInactive() {
  if (activeRenderPass) {
    MultiLogger::get()->error("Cannot record this command during a render pass.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkGraphicsPipelineBound() {
  if (!boundGraphicsPipeline) {
    
    MultiLogger::get()->error("Cannot record this command without a graphics pipeline bound.", stateString(state));
  }
}

void CommandBuffer::checkComputePipelineBound() {
  if (!boundComputePipeline) {
    
    MultiLogger::get()->error("Cannot record this command without a compute pipeline bound.", stateString(state));
  }
}

void CommandBuffer::begin(
    VkCommandBufferUsageFlags usage,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkFramebuffer framebuffer) {
  checkInitial();
  VkCommandBufferInheritanceInfo inheritInfo{};
  inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritInfo.renderPass = renderPass;
  inheritInfo.subpass = subpass;
  inheritInfo.framebuffer = framebuffer;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = usage;
  beginInfo.pInheritanceInfo =
      level == VK_COMMAND_BUFFER_LEVEL_SECONDARY ? &inheritInfo : nullptr;
  vkBeginCommandBuffer(commandBufferHandle, &beginInfo);
  state = State::Recording;
}

void CommandBuffer::end() { 
  checkRecording();
  checkRenderPassInactive();
  vkEndCommandBuffer(commandBufferHandle);
  state = State::Executable;
}

void CommandBuffer::bindGraphicsPipeline(std::shared_ptr<GraphicsPipeline> pipeline) {
  checkRecording();
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
  graphicsPipelines.push_back(std::move(pipeline));
}

void CommandBuffer::bindComputePipeline(std::shared_ptr<ComputePipeline> pipeline) {
  checkRecording();
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);
  computePipelines.push_back(std::move(pipeline));
}

void CommandBuffer::setViewport(
    uint32_t firstViewport,
    std::vector<VkViewport> viewports) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdSetViewport(
      commandBufferHandle,
      firstViewport,
      static_cast<uint32_t>(viewports.size()),
      viewports.data());
}

void CommandBuffer::setScissor(
    uint32_t firstScissor,
    std::vector<VkRect2D> scissors) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdSetScissor(
      commandBufferHandle,
      firstScissor,
      static_cast<uint32_t>(scissors.size()),
      scissors.data());
}

void CommandBuffer::setLineWidth(float lineWidth) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdSetLineWidth(commandBufferHandle, lineWidth);
}

void CommandBuffer::bindGraphicsDescriptorSets(
    std::shared_ptr<PipelineLayout> layout,
    uint32_t firstSet,
    const std::vector<std::shared_ptr<DescriptorSet>>& sets,
    const std::vector<uint32_t>& dynamicOffsets) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  std::vector<VkDescriptorSet> vkSets;
  vkSets.reserve(sets.size());
  for (auto& set : sets) {
    vkSets.push_back(*set);
  }
  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      *layout,
      firstSet,
      static_cast<uint32_t>(vkSets.size()),
      vkSets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
  pipelineLayouts.push_back(std::move(layout));
  descriptorSets.insert(descriptorSets.cend(),
    sets.begin(), sets.end());
}

void CommandBuffer::bindComputeDescriptorSets(
    std::shared_ptr<PipelineLayout> layout,
    uint32_t firstSet,
    const std::vector<std::shared_ptr<DescriptorSet>>& sets,
    const std::vector<uint32_t>& dynamicOffsets) {
  checkRecording();
  checkComputePipelineBound();
  std::vector<VkDescriptorSet> vkSets;
  vkSets.reserve(sets.size());
  for (auto& set : sets) {
    vkSets.push_back(*set);
  }
  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_COMPUTE,
      *layout,
      firstSet,
      static_cast<uint32_t>(vkSets.size()),
      vkSets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
  pipelineLayouts.push_back(std::move(layout));
  descriptorSets.insert(descriptorSets.cend(),
    sets.begin(), sets.end());
}

void CommandBuffer::bindIndexBuffer(
    std::shared_ptr<Buffer> buffer,
    uint32_t offset,
    VkIndexType indexType) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdBindIndexBuffer(commandBufferHandle, *buffer, offset, indexType);
  buffers.push_back(std::move(buffer));
}

void CommandBuffer::bindVertexBuffers(
    uint32_t firstBinding,
    const std::vector<std::shared_ptr<Buffer>>& buffers,
    const std::vector<VkDeviceSize>& offsets) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  std::vector<VkBuffer> vkBuffers;
  vkBuffers.reserve(buffers.size());
  for (auto& buffer : buffers) {
    vkBuffers.push_back(*buffer);
    this->buffers.push_back(std::move(buffer));
  }
  vkCmdBindVertexBuffers(
      commandBufferHandle,
      firstBinding,
      static_cast<uint32_t>(vkBuffers.size()),
      vkBuffers.data(),
      offsets.data());
}

void CommandBuffer::draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdDraw(
      commandBufferHandle,
      vertexCount,
      instanceCount,
      firstVertex,
      firstInstance);
}

void CommandBuffer::drawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    uint32_t vertexOffset,
    uint32_t firstInstance) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdDrawIndexed(
      commandBufferHandle,
      indexCount,
      instanceCount,
      firstIndex,
      vertexOffset,
      firstInstance);
}

void CommandBuffer::copyBuffer(
    std::shared_ptr<Buffer> srcBuffer,
    std::shared_ptr<Buffer> dstBuffer,
    std::vector<VkBufferCopy> regions) {
  vkCmdCopyBuffer(
      commandBufferHandle,
      *srcBuffer,
      *dstBuffer,
      static_cast<uint32_t>(regions.size()),
      regions.data());
  buffers.push_back(std::move(srcBuffer));
  buffers.push_back(std::move(dstBuffer));
}

void CommandBuffer::copyImage(
    std::shared_ptr<Image> srcImage,
    VkImageLayout srcImageLayout,
    std::shared_ptr<Image> dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkImageCopy>& regions) {
  vkCmdCopyImage(
      commandBufferHandle,
      *srcImage,
      srcImageLayout,
      *dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data());
  images.push_back(std::move(srcImage));
  images.push_back(std::move(dstImage));
}

void CommandBuffer::blitImage(
    std::shared_ptr<Image> srcImage,
    VkImageLayout srcImageLayout,
    std::shared_ptr<Image> dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkImageBlit>& regions,
    VkFilter filter) {
  vkCmdBlitImage(
      commandBufferHandle,
      *srcImage,
      srcImageLayout,
      *dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data(),
      filter);
  images.push_back(std::move(srcImage));
  images.push_back(std::move(dstImage));
}

void CommandBuffer::copyBufferToImage(
    std::shared_ptr<Buffer> srcBuffer,
    std::shared_ptr<Image> dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkBufferImageCopy>& regions) {
  vkCmdCopyBufferToImage(
      commandBufferHandle,
      *srcBuffer,
      *dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data());
  buffers.push_back(std::move(srcBuffer));
  images.push_back(std::move(dstImage));
}

void CommandBuffer::copyImageToBuffer(
    std::shared_ptr<Image> srcImage,
    VkImageLayout srcImageLayout,
    std::shared_ptr<Buffer> dstBuffer,
    const std::vector<VkBufferImageCopy>& regions) {
  vkCmdCopyImageToBuffer(
      commandBufferHandle,
      *srcImage,
      srcImageLayout,
      *dstBuffer,
      static_cast<uint32_t>(regions.size()),
      regions.data());
  images.push_back(std::move(srcImage));
  buffers.push_back(std::move(dstBuffer));
}

void CommandBuffer::pipelineBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    const std::vector<VkMemoryBarrier>& memoryBarriers,
    const std::vector<VkBufferMemoryBarrier>& bufferBarriers,
    const std::vector<VkImageMemoryBarrier>& imageBarriers) {
  vkCmdPipelineBarrier(
      commandBufferHandle,
      srcStageMask,
      dstStageMask,
      dependencyFlags,
      static_cast<uint32_t>(memoryBarriers.size()),
      memoryBarriers.data(),
      static_cast<uint32_t>(bufferBarriers.size()),
      bufferBarriers.data(),
      static_cast<uint32_t>(imageBarriers.size()),
      imageBarriers.data());
}

void CommandBuffer::pushConstants(
    std::shared_ptr<PipelineLayout> layout,
    VkShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    void* pValues) {
  checkRecording();
  vkCmdPushConstants(
      commandBufferHandle, *layout, stageFlags, offset, size, pValues);
  pipelineLayouts.push_back(std::move(layout));
}

void CommandBuffer::beginRenderPass(
    std::shared_ptr<RenderPass> renderPass,
    std::shared_ptr<Framebuffer> framebuffer,
    VkRect2D renderArea,
    const std::vector<VkClearValue>& clearValues,
    VkSubpassContents contents) {
  checkRecording();
  checkRenderPassInactive();
  VkRenderPassBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.renderPass = *renderPass;
  beginInfo.framebuffer = *framebuffer;
  beginInfo.renderArea = std::move(renderArea);
  beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  beginInfo.pClearValues = clearValues.data();
  vkCmdBeginRenderPass(commandBufferHandle, &beginInfo, contents);
  activeRenderPass = renderPass;
  renderPasses.push_back(std::move(renderPass));
  framebuffers.push_back(std::move(framebuffer));
}

void CommandBuffer::nextSubpass(VkSubpassContents contents) {
  checkRecording();
  checkRenderPassActive();
  if (level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    MultiLogger::get()->error("Attempt to call next subpass from secondary cmd buffer.");
    throw std::runtime_error("");
  }
  vkCmdNextSubpass(commandBufferHandle, contents);
}

void CommandBuffer::endRenderPass() {
  checkRecording();
  checkRenderPassActive();
  vkCmdEndRenderPass(commandBufferHandle);
  activeRenderPass.reset();
  boundGraphicsPipeline.reset();
}

void CommandBuffer::executeCommands(
    const std::vector<std::shared_ptr<CommandBuffer>>& commandBuffers) {
  checkRecording();
  if (level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    MultiLogger::get()->error("Attempt to execute secondary command buffer from secondary command buffer.");
    throw std::runtime_error("");
  }
  std::vector<VkCommandBuffer> vkCmds;
  vkCmds.reserve(commandBuffers.size());
  for (auto& cmd : commandBuffers) {
    vkCmds.push_back(*cmd);
    this->commandBuffers.push_back(std::move(cmd));
  }
  vkCmdExecuteCommands(
      commandBufferHandle,
      static_cast<uint32_t>(vkCmds.size()),
      vkCmds.data());
}

CommandPool::CommandPool(VkDevice device,
    uint32_t queueIndex,
    VkCommandBufferLevel level,
    bool transient)
    : device(device),
    level(level) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = queueIndex;
  createInfo.flags = 
    transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;
  auto poolResult =
      vkCreateCommandPool(device, &createInfo, nullptr, &poolHandle);
}

std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer() {
  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = static_cast<uint32_t>(1);
  allocateInfo.commandPool = poolHandle;
  allocateInfo.level = level;
  vkAllocateCommandBuffers(device, &allocateInfo, &cmd);
  cmdBuffers.push_back(std::make_shared<CommandBuffer>(cmd, level));
  return cmdBuffers.back();
}

void CommandPool::reset(bool releaseResources) {
  vkResetCommandPool(
      device,
      poolHandle,
      releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
  for (auto& cmd : cmdBuffers) {
    cmd->state = CommandBuffer::Initial;
  }
}

CommandPool::~CommandPool() {
  if (poolHandle != VK_NULL_HANDLE) {
    for (auto& cmd : cmdBuffers) {
      cmd->state = CommandBuffer::Invalid;
    }
    vkDestroyCommandPool(device, poolHandle, nullptr);
  }
}
}  // namespace vka