#include "CommandPool.hpp"
#include <range/v3/algorithm/for_each.hpp>
#include "Device.hpp"

namespace vka {
CommandBuffer::CommandBuffer(
    VkCommandBuffer commandBuffer,
    VkCommandBufferLevel level,
    bool transient)
    : commandBufferHandle(commandBuffer),
      level(level),
      transient(transient),
      state(Initial) {}

void CommandBuffer::checkInitial() {
  if (state != Initial) {
    MultiLogger::get()->error(
        "Incorrect cmd buffer state {}. Should be in initial state.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRecording() {
  if (state != Recording) {
    MultiLogger::get()->error(
        "Incorrect cmd buffer state {}. Should be in recording state.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkExecutable() {
  if (state != Executable) {
    MultiLogger::get()->error(
        "Incorrect cmd buffer state {}. Should be in executable state.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkPending() {
  if (state != Pending) {
    MultiLogger::get()->error(
        "Incorrect cmd buffer state {}. Should be in pending state.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRenderPassActive() {
  if (!activeRenderPass) {
    MultiLogger::get()->error(
        "Cannot record this command outside of a render pass.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkRenderPassInactive() {
  if (activeRenderPass) {
    MultiLogger::get()->error(
        "Cannot record this command during a render pass.", stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkGraphicsPipelineBound() {
  if (!boundGraphicsPipeline) {
    MultiLogger::get()->error(
        "Cannot record this command without a graphics pipeline bound.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::checkComputePipelineBound() {
  if (!boundComputePipeline) {
    MultiLogger::get()->error(
        "Cannot record this command without a compute pipeline bound.",
        stateString(state));
    throw std::runtime_error("");
  }
}

void CommandBuffer::cmdPending() { state = Pending; }

void CommandBuffer::cmdExecuted() {
  state = transient ? Invalid : Initial;
  for (auto& resourceVariant : dependentResources) {
    std::visit(
        [](auto& resource) { resource->cmdExecuted(); }, resourceVariant);
  }
  dependentResources.clear();
  activeRenderPass.reset();
  boundComputePipeline.reset();
  boundGraphicsPipeline.reset();
}

void CommandBuffer::begin(
    bool renderPassContinue,
    bool simultaneousUse,
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
  beginInfo.flags |=
      transient ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
  beginInfo.flags |=
      renderPassContinue ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0;
  beginInfo.flags |=
      simultaneousUse ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;
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

void CommandBuffer::bindGraphicsPipeline(
    std::shared_ptr<GraphicsPipeline> pipeline) {
  checkRecording();
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
  boundGraphicsPipeline = pipeline;
  dependentResources.emplace_back(std::move(pipeline));
}

void CommandBuffer::bindComputePipeline(
    std::shared_ptr<ComputePipeline> pipeline) {
  checkRecording();
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);
  boundComputePipeline = pipeline;
  dependentResources.emplace_back(std::move(pipeline));
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
  ranges::for_each(sets, [&](auto set) {
    vkSets.push_back(*set);
    dependentResources.emplace_back(set);
  });
  dependentResources.emplace_back(layout);

  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      *layout,
      firstSet,
      static_cast<uint32_t>(vkSets.size()),
      vkSets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
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
  ranges::for_each(sets, [&](auto set) {
    vkSets.push_back(*set);
    dependentResources.emplace_back(set);
  });
  dependentResources.emplace_back(layout);

  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_COMPUTE,
      *layout,
      firstSet,
      static_cast<uint32_t>(vkSets.size()),
      vkSets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
}

void CommandBuffer::bindIndexBuffer(
    std::shared_ptr<Buffer> buffer,
    uint32_t offset,
    VkIndexType indexType) {
  checkRecording();
  checkRenderPassActive();
  checkGraphicsPipelineBound();
  vkCmdBindIndexBuffer(commandBufferHandle, *buffer, offset, indexType);
  dependentResources.emplace_back(std::move(buffer));
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
  ranges::for_each(buffers, [&](auto buffer) {
    vkBuffers.push_back(*buffer);
    dependentResources.emplace_back(std::move(buffer));
  });

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
  dependentResources.emplace_back(std::move(srcBuffer));
  dependentResources.emplace_back(std::move(dstBuffer));
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
  dependentResources.emplace_back(std::move(srcImage));
  dependentResources.emplace_back(std::move(dstImage));
}

void CommandBuffer::blitImage(
    std::shared_ptr<Image> srcImage,
    std::shared_ptr<Image> dstImage,
    const std::vector<VkImageBlit>& regions,
    VkFilter filter) {
  vkCmdBlitImage(
      commandBufferHandle,
      *srcImage,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      *dstImage,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      static_cast<uint32_t>(regions.size()),
      regions.data(),
      filter);
  dependentResources.emplace_back(std::move(srcImage));
  dependentResources.emplace_back(std::move(dstImage));
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
  dependentResources.emplace_back(std::move(srcBuffer));
  dependentResources.emplace_back(std::move(dstImage));
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
  dependentResources.emplace_back(std::move(srcImage));
  dependentResources.emplace_back(std::move(dstBuffer));
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
  dependentResources.emplace_back(std::move(layout));
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
  dependentResources.emplace_back(renderPass);
  ranges::for_each(framebuffer->views, [&](auto view) {
    dependentResources.emplace_back(view);
  });
  dependentResources.emplace_back(framebuffer);
}

void CommandBuffer::nextSubpass(VkSubpassContents contents) {
  checkRecording();
  checkRenderPassActive();
  if (level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    MultiLogger::get()->error(
        "Attempt to call next subpass from secondary cmd buffer.");
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
    MultiLogger::get()->error(
        "Attempt to execute secondary command buffer from secondary command "
        "buffer.");
    throw std::runtime_error("");
  }

  std::vector<VkCommandBuffer> vkCmds;
  vkCmds.reserve(commandBuffers.size());
  ranges::for_each(commandBuffers, [&](auto cmd) {
    vkCmds.push_back(*cmd);
    dependentResources.emplace_back(std::move(cmd));
  });

  vkCmdExecuteCommands(
      commandBufferHandle, static_cast<uint32_t>(vkCmds.size()), vkCmds.data());
}

void CommandBuffer::recordGlobalBarrier(
    std::vector<ThsvsAccessType> previous,
    std::vector<ThsvsAccessType> next) {
  ThsvsGlobalBarrier thBarrier{};
  thBarrier.prevAccessCount = static_cast<uint32_t>(previous.size());
  thBarrier.pPrevAccesses = previous.data();
  thBarrier.nextAccessCount = static_cast<uint32_t>(next.size());
  thBarrier.pNextAccesses = next.data();
  VkPipelineStageFlags src{};
  VkPipelineStageFlags dst{};
  VkMemoryBarrier barrier{};
  thsvsGetVulkanMemoryBarrier(thBarrier, &src, &dst, &barrier);
  pipelineBarrier(src, dst, 0, {barrier}, {}, {});
}

void CommandBuffer::recordImageBarrier(
    std::vector<ThsvsAccessType> previous,
    std::vector<ThsvsAccessType> next,
    std::shared_ptr<vka::Image> image,
    ThsvsImageLayout newLayout,
    bool discardContents) {
  ThsvsImageBarrier thBarrier{};
  thBarrier.discardContents = VkBool32(discardContents);
  thBarrier.prevAccessCount = static_cast<uint32_t>(previous.size());
  thBarrier.pPrevAccesses = previous.data();
  thBarrier.nextAccessCount = static_cast<uint32_t>(next.size());
  thBarrier.pNextAccesses = next.data();
  thBarrier.image = *image;
  thBarrier.prevLayout = image->layout;
  image->layout = newLayout;
  thBarrier.nextLayout = newLayout;
  thBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,
                                0,
                                VK_REMAINING_MIP_LEVELS,
                                0,
                                VK_REMAINING_ARRAY_LAYERS};
  VkPipelineStageFlags src{};
  VkPipelineStageFlags dst{};
  VkImageMemoryBarrier barrier{};
  thsvsGetVulkanImageMemoryBarrier(thBarrier, &src, &dst, &barrier);
  pipelineBarrier(src, dst, 0, {}, {}, {barrier});
}

CommandPool::CommandPool(
    VkDevice device,
    uint32_t queueIndex,
    VkCommandBufferLevel level,
    bool transient)
    : device(device), level(level), transient(transient) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = queueIndex;
  createInfo.flags = transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;
  auto poolResult =
      vkCreateCommandPool(device, &createInfo, nullptr, &poolHandle);
  if (poolResult != VK_SUCCESS) {
    MultiLogger::get()->error("Error creating command pool!");
  }
}

std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer() {
  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = static_cast<uint32_t>(1);
  allocateInfo.commandPool = poolHandle;
  allocateInfo.level = level;
  vkAllocateCommandBuffers(device, &allocateInfo, &cmd);
  cmdBuffers.push_back(std::make_shared<CommandBuffer>(cmd, level, transient));
  return cmdBuffers.back();
}

void CommandPool::reset(bool releaseResources) {
  vkResetCommandPool(
      device,
      poolHandle,
      releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);

  ranges::for_each(
      cmdBuffers, [](auto cmd) { cmd->state = CommandBuffer::Initial; });
}

CommandPool::~CommandPool() {
  if (poolHandle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, poolHandle, nullptr);

    ranges::for_each(
        cmdBuffers, [](auto cmd) { cmd->state = CommandBuffer::Invalid; });
  }
}
}  // namespace vka