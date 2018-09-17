#include "RenderPass.hpp"
#include "Device.hpp"
#include "VulkanFunctionLoader.hpp"

namespace vka {
RenderPass::RenderPass(
    Device* device,
    std::vector<VkAttachmentDescription> attachments,
    std::vector<VkSubpassDescription> subpasses,
    std::vector<VkSubpassDependency> dependencies)
    : device(device) {
  VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  createInfo.pAttachments = attachments.data();
  createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
  createInfo.pSubpasses = subpasses.data();
  createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  createInfo.pDependencies = dependencies.data();

  vkCreateRenderPass(
      device->getHandle(), &createInfo, nullptr, &renderPassHandle);
}

RenderPass::~RenderPass() {
  vkDestroyRenderPass(device->getHandle(), renderPassHandle, nullptr);
}
}  // namespace vka