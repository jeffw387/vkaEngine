#include "RenderPass.hpp"
#include "Device.hpp"
#include <vulkan/vulkan.h>

namespace vka {
Subpass::Subpass(VkPipelineBindPoint bindPoint) : bindPoint(bindPoint){};

void Subpass::addInputRef(VkAttachmentReference inputRef) {
  inputRefs.push_back(inputRef);
}

void Subpass::addColorRef(VkAttachmentReference colorRef) {
  colorRefs.push_back(colorRef);
}

void Subpass::addResolveRef(VkAttachmentReference resolveRef) {
  resolveRefs.push_back(resolveRef);
}

void Subpass::setDepthRef(VkAttachmentReference depthRef) {
  this->depthRef = depthRef;
}

void Subpass::addPreserveRef(uint32_t preserveRef) {
  preserveRefs.push_back(preserveRef);
}

Subpass::operator VkSubpassDescription() {
  VkSubpassDescription result{};
  result.pipelineBindPoint = bindPoint;
  result.inputAttachmentCount = static_cast<uint32_t>(inputRefs.size());
  result.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
  result.preserveAttachmentCount = static_cast<uint32_t>(preserveRefs.size());
  result.pInputAttachments = inputRefs.data();
  result.pColorAttachments = colorRefs.data();
  result.pResolveAttachments = resolveRefs.data();
  result.pDepthStencilAttachment = &depthRef;
  result.pPreserveAttachments = preserveRefs.data();
  return result;
}

RenderPassCreateInfo::operator const VkRenderPassCreateInfo&() {
  subpassDescriptions.resize(subpasses.size());
  for (auto i = 0U; i < subpasses.size(); ++i) {
    subpassDescriptions[i] = subpasses[i];
  }
  createInfo =
      VkRenderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  createInfo.pAttachments = attachments.data();
  createInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
  createInfo.pSubpasses = subpassDescriptions.data();
  createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  createInfo.pDependencies = dependencies.data();

  return createInfo;
}

uint32_t RenderPassCreateInfo::addAttachmentDescription(
    VkAttachmentDescriptionFlags flags,
    VkFormat format,
    VkSampleCountFlagBits samples,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkAttachmentLoadOp stencilLoadOp,
    VkAttachmentStoreOp stencilStoreOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout) {
  auto result = static_cast<uint32_t>(attachments.size());
  attachments.emplace_back(VkAttachmentDescription{flags,
                                                   format,
                                                   samples,
                                                   loadOp,
                                                   storeOp,
                                                   stencilLoadOp,
                                                   stencilStoreOp,
                                                   initialLayout,
                                                   finalLayout});
  return result;
}

RenderPass::RenderPass(
    VkDevice device,
    const VkRenderPassCreateInfo& createInfo)
    : device(device) {
  vkCreateRenderPass(device, &createInfo, nullptr, &renderPassHandle);
}

Subpass* RenderPassCreateInfo::addGraphicsSubpass() {
  auto& subpass = subpasses.emplace_back(VK_PIPELINE_BIND_POINT_GRAPHICS);
  return &subpass;
}

Subpass* RenderPassCreateInfo::addComputeSubpass() {
  auto& subpass = subpasses.emplace_back(VK_PIPELINE_BIND_POINT_COMPUTE);
  return &subpass;
}

RenderPass::~RenderPass() {
  vkDestroyRenderPass(device, renderPassHandle, nullptr);
}
}  // namespace vka