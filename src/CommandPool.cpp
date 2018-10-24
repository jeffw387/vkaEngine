#include "CommandPool.hpp"
#include "Device.hpp"

namespace vka {

void CommandBuffer::begin(
    VkCommandBufferUsageFlags usage,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkFramebuffer framebuffer) {
  VkCommandBufferInheritanceInfo inheritInfo{};
  inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritInfo.renderPass = renderPass;
  inheritInfo.subpass = subpass;
  inheritInfo.framebuffer = framebuffer;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = usage;
  beginInfo.pInheritanceInfo = &inheritInfo;
  vkBeginCommandBuffer(commandBufferHandle, &beginInfo);
}

void CommandBuffer::end() { vkEndCommandBuffer(commandBufferHandle); }

void CommandBuffer::bindGraphicsPipeline(VkPipeline pipeline) {
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bindComputePipeline(VkPipeline pipeline) {
  vkCmdBindPipeline(
      commandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void CommandBuffer::setViewport(
    uint32_t firstViewport,
    std::vector<VkViewport> viewports) {
  vkCmdSetViewport(
      commandBufferHandle,
      firstViewport,
      static_cast<uint32_t>(viewports.size()),
      viewports.data());
}

void CommandBuffer::setScissor(
    uint32_t firstScissor,
    std::vector<VkRect2D> scissors) {
  vkCmdSetScissor(
      commandBufferHandle,
      firstScissor,
      static_cast<uint32_t>(scissors.size()),
      scissors.data());
}

void CommandBuffer::setLineWidth(float lineWidth) {
  vkCmdSetLineWidth(commandBufferHandle, lineWidth);
}

void CommandBuffer::bindGraphicsDescriptorSets(
    VkPipelineLayout layout,
    uint32_t firstSet,
    const std::vector<VkDescriptorSet>& sets,
    const std::vector<uint32_t>& dynamicOffsets) {
  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      layout,
      firstSet,
      static_cast<uint32_t>(sets.size()),
      sets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
}

void CommandBuffer::bindComputeDescriptorSets(
    VkPipelineLayout layout,
    uint32_t firstSet,
    const std::vector<VkDescriptorSet>& sets,
    const std::vector<uint32_t>& dynamicOffsets) {
  vkCmdBindDescriptorSets(
      commandBufferHandle,
      VK_PIPELINE_BIND_POINT_COMPUTE,
      layout,
      firstSet,
      static_cast<uint32_t>(sets.size()),
      sets.data(),
      static_cast<uint32_t>(dynamicOffsets.size()),
      dynamicOffsets.data());
}

void CommandBuffer::bindIndexBuffer(
    VkBuffer buffer,
    uint32_t offset,
    VkIndexType indexType) {
  vkCmdBindIndexBuffer(commandBufferHandle, buffer, offset, indexType);
}

void CommandBuffer::bindVertexBuffers(
    uint32_t firstBinding,
    const std::vector<VkBuffer>& buffers,
    const std::vector<VkDeviceSize>& offsets) {
  vkCmdBindVertexBuffers(
      commandBufferHandle,
      firstBinding,
      static_cast<uint32_t>(buffers.size()),
      buffers.data(),
      offsets.data());
}

void CommandBuffer::draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) {
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
  vkCmdDrawIndexed(
      commandBufferHandle,
      indexCount,
      instanceCount,
      firstIndex,
      vertexOffset,
      firstInstance);
}

void CommandBuffer::copyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    std::vector<VkBufferCopy> regions) {
  vkCmdCopyBuffer(
      commandBufferHandle,
      srcBuffer,
      dstBuffer,
      static_cast<uint32_t>(regions.size()),
      regions.data());
}

void CommandBuffer::copyImage(
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkImageCopy>& regions) {
  vkCmdCopyImage(
      commandBufferHandle,
      srcImage,
      srcImageLayout,
      dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data());
}

void CommandBuffer::blitImage(
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkImageBlit>& regions,
    VkFilter filter) {
  vkCmdBlitImage(
      commandBufferHandle,
      srcImage,
      srcImageLayout,
      dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data(),
      filter);
}

void CommandBuffer::copyBufferToImage(
    VkBuffer srcBuffer,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    const std::vector<VkBufferImageCopy>& regions) {
  vkCmdCopyBufferToImage(
      commandBufferHandle,
      srcBuffer,
      dstImage,
      dstImageLayout,
      static_cast<uint32_t>(regions.size()),
      regions.data());
}

void CommandBuffer::copyImageToBuffer(
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkBuffer dstBuffer,
    const std::vector<VkBufferImageCopy>& regions) {
  vkCmdCopyImageToBuffer(
      commandBufferHandle,
      srcImage,
      srcImageLayout,
      dstBuffer,
      static_cast<uint32_t>(regions.size()),
      regions.data());
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
    VkPipelineLayout layout,
    VkShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    void* pValues) {
  vkCmdPushConstants(
      commandBufferHandle, layout, stageFlags, offset, size, pValues);
}

void CommandBuffer::beginRenderPass(
    VkRenderPass renderPass,
    VkFramebuffer framebuffer,
    VkRect2D renderArea,
    const std::vector<VkClearValue>& clearValues,
    VkSubpassContents contents) {
  VkRenderPassBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.renderPass = renderPass;
  beginInfo.framebuffer = framebuffer;
  beginInfo.renderArea = renderArea;
  beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  beginInfo.pClearValues = clearValues.data();
  vkCmdBeginRenderPass(commandBufferHandle, &beginInfo, contents);
}

void CommandBuffer::nextSubpass(VkSubpassContents contents) {
  vkCmdNextSubpass(commandBufferHandle, contents);
}

void CommandBuffer::endRenderPass() { vkCmdEndRenderPass(commandBufferHandle); }

void CommandBuffer::executeCommands(
    const std::vector<CommandBuffer>& commandBuffers) {
  vkCmdExecuteCommands(
      commandBufferHandle,
      static_cast<uint32_t>(commandBuffers.size()),
      reinterpret_cast<const VkCommandBuffer*>(commandBuffers.data()));
}

CommandPool::CommandPool(VkDevice device, uint32_t queueIndex)
    : device(device) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = queueIndex;

  auto poolResult =
      vkCreateCommandPool(device, &createInfo, nullptr, &poolHandle);
}

std::unique_ptr<CommandBuffer> CommandPool::allocateCommandBuffer(
    VkCommandBufferLevel level) {
  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = static_cast<uint32_t>(1);
  allocateInfo.commandPool = poolHandle;
  allocateInfo.level = level;
  vkAllocateCommandBuffers(device, &allocateInfo, &cmd);
  return std::make_unique<CommandBuffer>(cmd);
}

CommandPool& CommandPool::operator=(CommandPool&& other) {
  if (this != &other) {
    device = other.device;
    poolHandle = other.poolHandle;
    other.device = {};
    other.poolHandle = {};
  }
  return *this;
}

CommandPool::CommandPool(CommandPool&& other) { *this = std::move(other); }

void CommandPool::reset(bool releaseResources) {
  vkResetCommandPool(
      device,
      poolHandle,
      releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
}

CommandPool::~CommandPool() {
  if (device != VK_NULL_HANDLE && poolHandle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, poolHandle, nullptr);
  }
}
}  // namespace vka