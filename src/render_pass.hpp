#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <map>
#include <tl/optional.hpp>

namespace vka {
struct render_pass {
  explicit render_pass(VkDevice device, VkRenderPass renderPass)
      : m_device(device), m_renderPass(renderPass) {}
  render_pass(const render_pass&) = delete;
  render_pass(render_pass&&) = default;
  render_pass& operator=(const render_pass&) = delete;
  render_pass& operator=(render_pass&&) = default;
  ~render_pass() { vkDestroyRenderPass(m_device, m_renderPass, nullptr); }
  operator VkRenderPass() const noexcept { return m_renderPass; }

private:
  VkDevice m_device = {};
  VkRenderPass m_renderPass = {};
};

struct attachment_builder {
  VkAttachmentDescription build() {
    m_description.samples = m_samples;
    return m_description;
  }

  attachment_builder& flag(VkAttachmentDescriptionFlagBits attachmentFlag) {
    m_description.flags |= attachmentFlag;
    return *this;
  }

  attachment_builder& samples(VkSampleCountFlagBits samplesFlag) {
    m_samples = samplesFlag;
    return *this;
  }

  attachment_builder& loadOp(VkAttachmentLoadOp op) {
    m_description.loadOp = op;
    return *this;
  }

  attachment_builder& storeOp(VkAttachmentStoreOp op) {
    m_description.storeOp = op;
    return *this;
  }

  attachment_builder& stencilLoadOp(VkAttachmentLoadOp op) {
    m_description.stencilLoadOp = op;
    return *this;
  }

  attachment_builder& stencilStoreOp(VkAttachmentStoreOp op) {
    m_description.stencilStoreOp = op;
    return *this;
  }

  attachment_builder& format(VkFormat attachmentFormat) {
    m_description.format = attachmentFormat;
    return *this;
  }

  attachment_builder& initial_layout(VkImageLayout layout) {
    m_description.initialLayout = layout;
    return *this;
  }

  attachment_builder& final_layout(VkImageLayout layout) {
    m_description.finalLayout = layout;
    return *this;
  }

private:
  VkAttachmentDescription m_description = {};
  VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
};

struct subpass {
  explicit subpass(
      VkPipelineBindPoint bindPoint,
      std::vector<VkAttachmentReference> inputAttachments,
      std::vector<VkAttachmentReference> colorAttachments,
      std::vector<VkAttachmentReference> resolveAttachments,
      tl::optional<VkAttachmentReference> depthAttachment,
      std::vector<uint32_t> preserveAttachments)
      : m_inputAttachments(inputAttachments),
        m_colorAttachments(colorAttachments),
        m_resolveAttachments(resolveAttachments),
        m_depthAttachment(depthAttachment),
        m_preserveAttachments(preserveAttachments) {
    m_description.pipelineBindPoint = bindPoint;
    m_description.inputAttachmentCount =
        static_cast<uint32_t>(m_inputAttachments.size());
    m_description.pInputAttachments = m_inputAttachments.data();
    m_description.colorAttachmentCount =
        static_cast<uint32_t>(m_colorAttachments.size());
    m_description.pColorAttachments = m_colorAttachments.data();
    m_description.pResolveAttachments = m_resolveAttachments.data();
    m_description.pDepthStencilAttachment = &m_depthAttachment.value();
    m_description.preserveAttachmentCount =
        static_cast<uint32_t>(m_preserveAttachments.size());
    m_description.pPreserveAttachments = m_preserveAttachments.data();
  }
  operator VkSubpassDescription() const noexcept { return m_description; }

private:
  VkSubpassDescription m_description = {};
  std::vector<VkAttachmentReference> m_inputAttachments;
  std::vector<VkAttachmentReference> m_colorAttachments;
  std::vector<VkAttachmentReference> m_resolveAttachments;
  tl::optional<VkAttachmentReference> m_depthAttachment;
  std::vector<uint32_t> m_preserveAttachments;
};

struct subpass_builder {
  subpass build() {
    return subpass{m_bindPoint,
                   m_inputAttachments,
                   m_colorAttachments,
                   m_resolveAttachments,
                   m_depthAttachment,
                   m_preserveAttachments};
  }

  subpass_builder& input_attachment(uint32_t index, VkImageLayout layout) {
    m_inputAttachments.push_back({index, layout});
    return *this;
  }

  subpass_builder& color_attachment(uint32_t index, VkImageLayout layout) {
    m_colorAttachments.push_back({index, layout});
    return *this;
  }

  subpass_builder& resolve_attachment(uint32_t index, VkImageLayout layout) {
    m_resolveAttachments.push_back({index, layout});
    return *this;
  }

  subpass_builder& depth_attachment(uint32_t index, VkImageLayout layout) {
    m_depthAttachment = VkAttachmentReference{index, layout};
    return *this;
  }

  subpass_builder& preserveAttachment(uint32_t index) {
    m_preserveAttachments.push_back(index);
    return *this;
  }

private:
  VkPipelineBindPoint m_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  std::vector<VkAttachmentReference> m_inputAttachments;
  std::vector<VkAttachmentReference> m_colorAttachments;
  std::vector<VkAttachmentReference> m_resolveAttachments;
  tl::optional<VkAttachmentReference> m_depthAttachment;
  std::vector<uint32_t> m_preserveAttachments;
};

struct subpass_dependency {
  operator VkSubpassDependency() { return m_dependency; }

  subpass_dependency& subpasses(uint32_t from, uint32_t to) {
    m_dependency.srcSubpass = from;
    m_dependency.dstSubpass = to;
    return *this;
  }

  subpass_dependency& source_stage(VkPipelineStageFlagBits stage) {
    m_dependency.srcStageMask |= stage;
    return *this;
  }

  subpass_dependency& destination_stage(VkPipelineStageFlagBits stage) {
    m_dependency.dstStageMask |= stage;
    return *this;
  }

  subpass_dependency& source_access(VkAccessFlagBits access) {
    m_dependency.srcAccessMask |= access;
    return *this;
  }

  subpass_dependency& destination_access(VkAccessFlagBits access) {
    m_dependency.dstAccessMask |= access;
    return *this;
  }

  subpass_dependency& dependency_type(VkDependencyFlagBits type) {
    m_dependency.dependencyFlags |= type;
    return *this;
  }

private:
  VkSubpassDependency m_dependency = {};
};

struct render_pass_builder {
  tl::expected<std::unique_ptr<render_pass>, VkResult> build(VkDevice device) {
    VkRenderPassCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    createInfo.attachmentCount =
        static_cast<uint32_t>(m_attachmentDescriptions.size());
    createInfo.subpassCount =
        static_cast<uint32_t>(m_subpassDescriptions.size());
    createInfo.dependencyCount = static_cast<uint32_t>(m_dependencies.size());
    createInfo.pAttachments = m_attachmentDescriptions.data();
    createInfo.pSubpasses = m_subpassDescriptions.data();
    createInfo.pDependencies = m_dependencies.data();

    VkRenderPass renderPass = {};
    auto result = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<render_pass>(device, renderPass);
  }

  render_pass_builder& add_attachment(VkAttachmentDescription description) {
    m_attachmentDescriptions.push_back(std::move(description));
    return *this;
  }

  render_pass_builder& add_subpass(VkSubpassDescription description) {
    m_subpassDescriptions.push_back(std::move(description));
    return *this;
  }

  render_pass_builder& add_dependency(VkSubpassDependency dependency) {
    m_dependencies.push_back(std::move(dependency));
    return *this;
  }

private:
  std::vector<VkAttachmentDescription> m_attachmentDescriptions;
  std::vector<VkSubpassDescription> m_subpassDescriptions;
  std::vector<VkSubpassDependency> m_dependencies;
};
}  // namespace vka