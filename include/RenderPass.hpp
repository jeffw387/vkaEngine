#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <memory>

namespace vka {
class Device;

class Subpass {
public:
  Subpass(VkPipelineBindPoint);
  void addInputRef(VkAttachmentReference);
  void addColorRef(VkAttachmentReference);
  void addResolveRef(VkAttachmentReference);
  void setDepthRef(VkAttachmentReference);
  void addPreserveRef(uint32_t);
  operator VkSubpassDescription();

private:
  VkPipelineBindPoint bindPoint;
  std::vector<VkAttachmentReference> inputRefs;
  std::vector<VkAttachmentReference> colorRefs;
  std::vector<VkAttachmentReference> resolveRefs;
  VkAttachmentReference depthRef;
  std::vector<uint32_t> preserveRefs;
};

class RenderPassCreateInfo {
public:
  uint32_t addAttachmentDescription(
      VkAttachmentDescriptionFlags flags,
      VkFormat format,
      VkSampleCountFlagBits samples,
      VkAttachmentLoadOp loadOp,
      VkAttachmentStoreOp storeOp,
      VkAttachmentLoadOp stencilLoadOp,
      VkAttachmentStoreOp stencilStoreOp,
      VkImageLayout initialLayout,
      VkImageLayout finalLayout);

  Subpass* addGraphicsSubpass();
  Subpass* addComputeSubpass();

  void addSubpassDependency(
      Subpass* srcSubpass,
      Subpass* dstSubpass,
      VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      VkAccessFlags srcAccessMask,
      VkAccessFlags dstAccessMask,
      VkDependencyFlags dependencyFlags);

  operator const VkRenderPassCreateInfo&();

private:
  std::vector<VkAttachmentDescription> attachments;
  std::vector<std::unique_ptr<Subpass>> subpasses;
  std::vector<VkSubpassDescription> subpassDescriptions;
  std::vector<VkSubpassDependency> dependencies;
  VkRenderPassCreateInfo createInfo;
};

class RenderPass {
public:
  RenderPass() = default;
  RenderPass(VkDevice device, const VkRenderPassCreateInfo& createInfo);
  RenderPass(RenderPass&&) = default;
  RenderPass& operator=(RenderPass&&) = default;
  RenderPass(const RenderPass&) = delete;
  RenderPass& operator=(const RenderPass&) = delete;
  ~RenderPass();

  operator VkRenderPass() { return renderPassHandle; }

private:
  VkDevice device;
  VkRenderPass renderPassHandle;
};
}  // namespace vka