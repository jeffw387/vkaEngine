#pragma once
#include "VulkanFunctionLoader.hpp"
#include <vector>

namespace vka {
class Device;

class RenderPassCreateInfo {
public:
  void addAttachmentDescription(
      VkAttachmentDescriptionFlags flags,
      VkFormat format,
      VkSampleCountFlagBits samples,
      VkAttachmentLoadOp loadOp,
      VkAttachmentStoreOp storeOp,
      VkAttachmentLoadOp stencilLoadOp,
      VkAttachmentStoreOp stencilStoreOp,
      VkImageLayout initialLayout,
      VkImageLayout finalLayout);

  void addSubpassDescription(
      VkPipelineBindPoint bindPoint,
      std::vector<VkAttachmentReference> inputAttachments,
      std::vector<VkAttachmentReference> colorAttachments,
      std::vector<VkAttachmentReference> resolveAttachments,
      VkAttachmentReference depthAttachment,
      std::vector<uint32_t> preserveAttachments);

  void addSubpassDependency(
      uint32_t srcSubpass,
      uint32_t dstSubpass,
      VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      VkAccessFlags srcAccessMask,
      VkAccessFlags dstAccessMask,
      VkDependencyFlags dependencyFlags);

  operator const VkRenderPassCreateInfo&();

private:
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkSubpassDescription> subpasses;
  std::vector<VkSubpassDependency> dependencies;
  VkRenderPassCreateInfo createInfo;
};
class RenderPass {
public:
  RenderPass() = delete;
  RenderPass(
      Device* device,
      std::vector<VkAttachmentDescription> attachments,
      std::vector<VkSubpassDescription> subpasses,
      std::vector<VkSubpassDependency> dependencies);
  RenderPass(RenderPass&&) = default;
  RenderPass& operator=(RenderPass&&) = default;
  RenderPass(const RenderPass&) = delete;
  RenderPass& operator=(const RenderPass&) = delete;
  ~RenderPass();

private:
  Device* device;
  VkRenderPass renderPassHandle;
};
}  // namespace vka