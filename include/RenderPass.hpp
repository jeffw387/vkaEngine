#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <optional>
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
  std::optional<VkAttachmentReference> depthRef;
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
  RenderPass(VkDevice device, const VkRenderPassCreateInfo& createInfo);
  ~RenderPass();
  void cmdExecuted(){};

  operator VkRenderPass() { return renderPassHandle; }

private:
  VkDevice device;
  VkRenderPass renderPassHandle;
};
}  // namespace vka