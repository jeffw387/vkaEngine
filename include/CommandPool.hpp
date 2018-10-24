#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace vka {
class Device;

class CommandBuffer {
public:
  CommandBuffer(VkCommandBuffer commandBuffer)
      : commandBufferHandle(commandBuffer) {}
  CommandBuffer(CommandBuffer&&) = default;
  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer& operator=(CommandBuffer&&) = default;
  CommandBuffer& operator=(const CommandBuffer&) = delete;
  ~CommandBuffer() = default;
  operator VkCommandBuffer() { return commandBufferHandle; }

  void begin(
      VkCommandBufferUsageFlags usage,
      VkRenderPass renderPass = VK_NULL_HANDLE,
      uint32_t subpass = 0,
      VkFramebuffer framebuffer = VK_NULL_HANDLE) {
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

  void end() { vkEndCommandBuffer(commandBufferHandle); }

  void bindGraphicsPipeline(VkPipeline pipeline) {
    vkCmdBindPipeline(
        commandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  void bindComputePipeline(VkPipeline pipeline) {
    vkCmdBindPipeline(
        commandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  }

  void setViewport(uint32_t firstViewport, std::vector<VkViewport> viewports) {
    vkCmdSetViewport(
        commandBufferHandle,
        firstViewport,
        static_cast<uint32_t>(viewports.size()),
        viewports.data());
  }

  void setScissor(uint32_t firstScissor, std::vector<VkRect2D> scissors) {
    vkCmdSetScissor(
        commandBufferHandle,
        firstScissor,
        static_cast<uint32_t>(scissors.size()),
        scissors.data());
  }

  void setLineWidth(float lineWidth) {
    vkCmdSetLineWidth(commandBufferHandle, lineWidth);
  }

  void bindGraphicsDescriptorSets(
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

  void bindComputeDescriptorSets(
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

  void
  bindIndexBuffer(VkBuffer buffer, uint32_t offset, VkIndexType indexType) {
    vkCmdBindIndexBuffer(commandBufferHandle, buffer, offset, indexType);
  }

  void bindVertexBuffers(
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

  void draw(
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

  void drawIndexed(
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

  void copyBuffer(
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

  void copyImage(
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

  void blitImage(
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

  void copyBufferToImage(
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

  void copyImageToBuffer(
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

  void pipelineBarrier(
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

  void pushConstants(
      VkPipelineLayout layout,
      VkShaderStageFlags stageFlags,
      uint32_t offset,
      uint32_t size,
      void* pValues) {
    vkCmdPushConstants(
        commandBufferHandle, layout, stageFlags, offset, size, pValues);
  }

  void beginRenderPass(
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

  void nextSubpass(VkSubpassContents contents) {
    vkCmdNextSubpass(commandBufferHandle, contents);
  }

  void endRenderPass() { vkCmdEndRenderPass(commandBufferHandle); }

  void executeCommands(const std::vector<CommandBuffer>& commandBuffers) {
    vkCmdExecuteCommands(
        commandBufferHandle,
        static_cast<uint32_t>(commandBuffers.size()),
        reinterpret_cast<const VkCommandBuffer*>(commandBuffers.data()));
  }

private:
  VkCommandBuffer commandBufferHandle;
};

class CommandPool {
public:
  CommandPool(VkDevice device, uint32_t gfxQueueIndex);
  CommandPool() = default;
  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;
  CommandPool(CommandPool&&);
  CommandPool& operator=(CommandPool&&);
  ~CommandPool();

  operator VkCommandPool() { return poolHandle; }
  std::vector<CommandBuffer> allocateCommandBuffers(
      size_t count,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  void reset(bool releaseResources = false);

private:
  VkDevice device;
  VkCommandPool poolHandle;
};
}  // namespace vka