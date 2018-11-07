#include "Framebuffer.hpp"

namespace vka {
Framebuffer::Framebuffer(
    VkDevice device,
    VkRenderPass renderPass,
    std::vector<std::shared_ptr<ImageView>> views,
    VkExtent2D extent) : device(device), views(std::move(views)) {
  std::vector<VkImageView> vkViews;
  vkViews.reserve(views.size());
  for (auto& view : this->views) {
    vkViews.push_back(*view);
  }
  VkFramebufferCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.attachmentCount = static_cast<uint32_t>(vkViews.size());
  createInfo.pAttachments = vkViews.data();
  createInfo.renderPass = renderPass;
  createInfo.width = extent.width;
  createInfo.height = extent.height;
  createInfo.layers = 1;
  vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer);
}

Framebuffer::~Framebuffer() {
  if (framebuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }
}

Framebuffer::operator VkFramebuffer() const noexcept {
  return framebuffer;
}
}