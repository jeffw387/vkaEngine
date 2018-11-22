#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <gsl-lite.hpp>
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "DescriptorPool.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "Logger.hpp"
#include "Fence.hpp"
#include "thsvs_simpler_vulkan_synchronization.h"

namespace vka {

class Device;
class CommandPool;
class CommandBuffer {
  friend class CommandPool;
  friend class Device;
  friend class Fence;

public:
  enum State { Initial, Recording, Executable, Pending, Invalid };

  static std::string_view stateString(State state) noexcept {
    switch (state) {
      case Initial:
        return "Initial";
      case Recording:
        return "Recording";
      case Executable:
        return "Executable";
      case Pending:
        return "Pending";
      case Invalid:
      default:
        return "Invalid";
    }
  }

  CommandBuffer(
      VkCommandBuffer commandBuffer,
      VkCommandBufferLevel level,
      bool transient);
  operator VkCommandBuffer() { return commandBufferHandle; }

  void begin(
      bool renderPassContinue = false,
      bool simultaneousUse = false,
      VkRenderPass renderPass = VK_NULL_HANDLE,
      uint32_t subpass = 0,
      VkFramebuffer framebuffer = VK_NULL_HANDLE);
  void end();
  void bindGraphicsPipeline(std::shared_ptr<GraphicsPipeline> pipeline);
  void bindComputePipeline(std::shared_ptr<ComputePipeline> pipeline);
  void setViewport(uint32_t firstViewport, std::vector<VkViewport> viewports);
  void setScissor(uint32_t firstScissor, std::vector<VkRect2D> scissors);
  void setLineWidth(float lineWidth);
  void bindGraphicsDescriptorSets(
      std::shared_ptr<PipelineLayout> layout,
      uint32_t firstSet,
      const std::vector<std::shared_ptr<DescriptorSet>>& sets,
      const std::vector<uint32_t>& dynamicOffsets);
  void bindComputeDescriptorSets(
      std::shared_ptr<PipelineLayout> layout,
      uint32_t firstSet,
      const std::vector<std::shared_ptr<DescriptorSet>>& sets,
      const std::vector<uint32_t>& dynamicOffsets);
  void bindIndexBuffer(
      std::shared_ptr<Buffer> buffer,
      uint32_t offset,
      VkIndexType indexType);
  void bindVertexBuffers(
      uint32_t firstBinding,
      const std::vector<std::shared_ptr<Buffer>>& buffers,
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
      std::shared_ptr<Buffer> srcBuffer,
      std::shared_ptr<Buffer> dstBuffer,
      std::vector<VkBufferCopy> regions);
  void copyImage(
      std::shared_ptr<Image> srcImage,
      VkImageLayout srcImageLayout,
      std::shared_ptr<Image> dstImage,
      VkImageLayout dstImageLayout,
      const std::vector<VkImageCopy>& regions);
  void blitImage(
      std::shared_ptr<Image> srcImage,
      std::shared_ptr<Image> dstImage,
      const std::vector<VkImageBlit>& regions,
      VkFilter filter);
  void copyBufferToImage(
      std::shared_ptr<Buffer> srcBuffer,
      std::shared_ptr<Image> dstImage,
      VkImageLayout dstImageLayout,
      const std::vector<VkBufferImageCopy>& regions);
  void copyImageToBuffer(
      std::shared_ptr<Image> srcImage,
      VkImageLayout srcImageLayout,
      std::shared_ptr<Buffer> dstBuffer,
      const std::vector<VkBufferImageCopy>& regions);
  void pipelineBarrier(
      VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      VkDependencyFlags dependencyFlags,
      const std::vector<VkMemoryBarrier>& memoryBarriers,
      const std::vector<VkBufferMemoryBarrier>& bufferBarriers,
      const std::vector<VkImageMemoryBarrier>& imageBarriers);
  void pushConstants(
      std::shared_ptr<PipelineLayout> layout,
      VkShaderStageFlags stageFlags,
      uint32_t offset,
      uint32_t size,
      void* pValues);
  void beginRenderPass(
      std::shared_ptr<RenderPass> renderPass,
      std::shared_ptr<Framebuffer> framebuffer,
      VkRect2D renderArea,
      const std::vector<VkClearValue>& clearValues,
      VkSubpassContents contents);
  void nextSubpass(VkSubpassContents contents);
  void endRenderPass();
  void executeCommands(
      const std::vector<std::shared_ptr<CommandBuffer>>& commandBuffers);

  void recordGlobalBarrier(
      std::vector<ThsvsAccessType> previous,
      std::vector<ThsvsAccessType> next);

  void recordImageBarrier(
      std::vector<ThsvsAccessType> previous,
      std::vector<ThsvsAccessType> next,
      std::shared_ptr<vka::Image> image,
      ThsvsImageLayout newLayout,
      bool discardContents = false);

  template <typename T>
  void recordBufferUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Buffer> staging,
      std::shared_ptr<vka::Buffer> buffer,
      VkDeviceSize offset) {
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    VkBufferCopy bufferCopy{0, offset, data.size_bytes()};
    copyBuffer(staging, buffer, {bufferCopy});
  }

  template <typename T>
  void recordImageUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Buffer> staging,
      std::shared_ptr<vka::Image> image,
      VkOffset2D imageOffset,
      VkExtent2D imageExtent) {
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    staging->unmap();

    VkImageSubresourceLayers imageSubresource = {};
    imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = 1;
    imageSubresource.mipLevel = 0;

    VkBufferImageCopy copy{};
    copy.imageOffset = {imageOffset.x, imageOffset.y, 0};
    copy.imageExtent = {imageExtent.width, imageExtent.height, 1};
    copy.imageSubresource = imageSubresource;
    copyBufferToImage(
        staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy});
  }

  template <typename T>
  void recordImageArrayUpload(
      gsl::span<T> data,
      std::shared_ptr<vka::Buffer> staging,
      std::shared_ptr<vka::Image> image) {
    void* stagePtr = staging->map();
    std::memcpy(stagePtr, data.data(), data.size_bytes());
    staging->flush();
    staging->unmap();

    VkImageSubresourceLayers imageSubresource = {};
    imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.layerCount = image->layerCount;
    imageSubresource.mipLevel = 0;

    VkBufferImageCopy copy{};
    copy.imageOffset = {};
    copy.imageExtent = image->extent;
    copy.imageSubresource = imageSubresource;
    copyBufferToImage(
        staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy});
  }

  void recordImageBlit(
      std::shared_ptr<vka::Image> source,
      std::shared_ptr<vka::Image> destination) {
    VkImageBlit blit{};
    blit.srcSubresource.aspectMask =
        static_cast<VkImageAspectFlags>(vka::ImageAspect::Color);
    blit.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    blit.dstSubresource.aspectMask =
        static_cast<VkImageAspectFlags>(vka::ImageAspect::Color);
    blit.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;

    blit.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    blitImage(source, destination, {blit}, VK_FILTER_LINEAR);
  }

  State currentState() const noexcept;

private:
  VkCommandBuffer commandBufferHandle = {};
  VkCommandBufferLevel level = {};
  bool transient = {};
  State state = {};

  using ResourceVariant = std::variant<
      std::shared_ptr<GraphicsPipeline>,
      std::shared_ptr<ComputePipeline>,
      std::shared_ptr<PipelineLayout>,
      std::shared_ptr<DescriptorSet>,
      std::shared_ptr<Image>,
      std::shared_ptr<ImageView>,
      std::shared_ptr<Buffer>,
      std::shared_ptr<RenderPass>,
      std::shared_ptr<Framebuffer>,
      std::shared_ptr<CommandBuffer>>;

  std::vector<ResourceVariant> dependentResources;

  std::shared_ptr<RenderPass> activeRenderPass;
  std::shared_ptr<ComputePipeline> boundComputePipeline;
  std::shared_ptr<GraphicsPipeline> boundGraphicsPipeline;

public:
  void cmdPending();
  void cmdExecuted();

private:
  void checkInitial();
  void checkRecording();
  void checkExecutable();
  void checkPending();
  void checkRenderPassActive();
  void checkRenderPassInactive();
  void checkGraphicsPipelineBound();
  void checkComputePipelineBound();
};

class CommandPool {
public:
  CommandPool(
      VkDevice device,
      uint32_t queueIndex,
      VkCommandBufferLevel level,
      bool transient);
  ~CommandPool();

  operator VkCommandPool() { return poolHandle; }
  std::shared_ptr<CommandBuffer> allocateCommandBuffer();
  void reset(bool releaseResources = false);
  VkCommandBufferLevel poolLevel() const noexcept;

private:
  VkDevice device = {};
  VkCommandPool poolHandle = {};
  VkCommandBufferLevel level = {};
  bool transient = {};
  std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;
};
}  // namespace vka