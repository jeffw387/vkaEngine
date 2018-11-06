#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace vka {
class Device;

class CommandBuffer {
public:
  CommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferLevel level)
      : commandBufferHandle(commandBuffer), level(level) {}
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
      VkFramebuffer framebuffer = VK_NULL_HANDLE);
  void end();
  void bindGraphicsPipeline(VkPipeline pipeline);
  void bindComputePipeline(VkPipeline pipeline);
  void setViewport(uint32_t firstViewport, std::vector<VkViewport> viewports);
  void setScissor(uint32_t firstScissor, std::vector<VkRect2D> scissors);
  void setLineWidth(float lineWidth);
  void bindGraphicsDescriptorSets(
      VkPipelineLayout layout,
      uint32_t firstSet,
      const std::vector<VkDescriptorSet>& sets,
      const std::vector<uint32_t>& dynamicOffsets);
  void bindComputeDescriptorSets(
      VkPipelineLayout layout,
      uint32_t firstSet,
      const std::vector<VkDescriptorSet>& sets,
      const std::vector<uint32_t>& dynamicOffsets);
  void bindIndexBuffer(VkBuffer buffer, uint32_t offset, VkIndexType indexType);
  void bindVertexBuffers(
      uint32_t firstBinding,
      const std::vector<VkBuffer>& buffers,
      const std::vector<VkDeviceSize>& offsets);
  void draw(
      uint32_t vertexCount,
      uint32_t instanceCount,
      uint32_t firstVertex,
      uint32_t firstInstance);
  void drawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount,
      uint32_t firstIndex,
      uint32_t vertexOffset,
      uint32_t firstInstance);
  void copyBuffer(
      VkBuffer srcBuffer,
      VkBuffer dstBuffer,
      std::vector<VkBufferCopy> regions);
  void copyImage(
      VkImage srcImage,
      VkImageLayout srcImageLayout,
      VkImage dstImage,
      VkImageLayout dstImageLayout,
      const std::vector<VkImageCopy>& regions);
  void blitImage(
      VkImage srcImage,
      VkImageLayout srcImageLayout,
      VkImage dstImage,
      VkImageLayout dstImageLayout,
      const std::vector<VkImageBlit>& regions,
      VkFilter filter);
  void copyBufferToImage(
      VkBuffer srcBuffer,
      VkImage dstImage,
      VkImageLayout dstImageLayout,
      const std::vector<VkBufferImageCopy>& regions);
  void copyImageToBuffer(
      VkImage srcImage,
      VkImageLayout srcImageLayout,
      VkBuffer dstBuffer,
      const std::vector<VkBufferImageCopy>& regions);
  void pipelineBarrier(
      VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      VkDependencyFlags dependencyFlags,
      const std::vector<VkMemoryBarrier>& memoryBarriers,
      const std::vector<VkBufferMemoryBarrier>& bufferBarriers,
      const std::vector<VkImageMemoryBarrier>& imageBarriers);
  void pushConstants(
      VkPipelineLayout layout,
      VkShaderStageFlags stageFlags,
      uint32_t offset,
      uint32_t size,
      void* pValues);
  void beginRenderPass(
      VkRenderPass renderPass,
      VkFramebuffer framebuffer,
      VkRect2D renderArea,
      const std::vector<VkClearValue>& clearValues,
      VkSubpassContents contents);
  void nextSubpass(VkSubpassContents contents);
  void endRenderPass();
  void executeCommands(const std::vector<CommandBuffer>& commandBuffers);

private:
  VkCommandBuffer commandBufferHandle = {};
  VkCommandBufferLevel level = {};
};

class CommandPool {
public:
  CommandPool(VkDevice device, uint32_t queueIndex);
  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;
  CommandPool(CommandPool&&);
  CommandPool& operator=(CommandPool&&);
  ~CommandPool();

  operator VkCommandPool() { return poolHandle; }
  std::unique_ptr<CommandBuffer> allocateCommandBuffer(
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  void reset(bool releaseResources = false);

private:
  VkDevice device;
  VkCommandPool poolHandle;
};
}  // namespace vka