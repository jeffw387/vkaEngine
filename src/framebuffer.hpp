#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>

namespace vka {
struct framebuffer {
  explicit framebuffer(VkDevice device, VkFramebuffer framebufferHandle)
  : m_device(device), m_framebuffer(framebufferHandle) {}
  
  framebuffer(const framebuffer&) = delete;
  framebuffer(framebuffer&&) = default;
  framebuffer& operator=(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) = default;
  
  ~framebuffer() noexcept {
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
  }

  operator VkFramebuffer() const noexcept { return m_framebuffer; }
private:
  VkDevice m_device = {};
  VkFramebuffer m_framebuffer = {};
};

struct framebuffer_builder {
  tl::expected<std::unique_ptr<framebuffer>, VkResult> build(VkDevice device) {
    VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    createInfo.renderPass = m_renderPass;
    createInfo.width = m_width;
    createInfo.height = m_height;
    createInfo.layers = m_layers;
    createInfo.attachmentCount = static_cast<uint32_t>(m_views.size());
    createInfo.pAttachments = m_views.data();

    VkFramebuffer framebufferHandle = {};
    auto result = vkCreateFramebuffer(device, &createInfo, nullptr, &framebufferHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<framebuffer>(device, framebufferHandle);
  }

  framebuffer_builder& render_pass(VkRenderPass renderPass) {
    m_renderPass = renderPass;
    return *this;
  }

  framebuffer_builder& dimensions(uint32_t width = 1, uint32_t height = 1, uint32_t layers = 1) {
    m_width = width;
    m_height = height;
    m_layers = layers;
    return *this;
  }

  framebuffer_builder& attachments(std::vector<VkImageView> views) {
    m_views = std::move(views);
    return *this;
  }
  
private:
  VkRenderPass m_renderPass = {};
  uint32_t m_width = {};
  uint32_t m_height = {};
  uint32_t m_layers = {};
  std::vector<VkImageView> m_views = {};
};
}