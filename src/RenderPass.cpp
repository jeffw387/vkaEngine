#include "RenderPass.hpp"
#include "Device.hpp"
#include "VulkanFunctionLoader.hpp"

namespace vka {
RenderPassCreateInfo::operator const VkRenderPassCreateInfo&() {
  createInfo =
      VkRenderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  createInfo.pAttachments = attachments.data();
  createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
  createInfo.pSubpasses = subpasses.data();
  createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  createInfo.pDependencies = dependencies.data();

  return createInfo;
}
RenderPass::RenderPass(
    VkDevice device,
    const VkRenderPassCreateInfo& createInfo)
    : device(device) {
  vkCreateRenderPass(device, &createInfo, nullptr, &renderPassHandle);
}

RenderPass::~RenderPass() {
  vkDestroyRenderPass(device, renderPassHandle, nullptr);
}
}  // namespace vka