#pragma once
#include <memory>
#include <variant>
#include <vector>
#include "VulkanFunctionLoader.hpp"
#include "braintree.hpp"

namespace bt = BrainTree;

// ||-RenderPass-||
// --Attachments
// --Subpasses
// --Dependencies

// ||-Subpass-||
// --Render Targets:
// ---Format
// ---Transient

// --Depth Attachment:
// ---Format
// ---Transient

// --Input Attachments:
// ---Format
// ---Size
// ---Transient

// subpasses default to separate renderpasses
// if a pass doesn't use an input attachment that was written to in the previous
// pass, the passes can be combined into a renderpass

// frame graph compilation:
// condense render passes to subpasses where possible
// generate workflow:
// --create graph (virtualized) with all attachments, resource transitions,
// barriers, renderpasses, subpasses
//

// StateUpdated
// Current UpdateState: 0
// Current RenderState: None

struct RenderContext {
  VkDevice device;
  VkCommandPool cmdPool;

  enum RenderStage {
    DataValidation,
    ResourceValidation,
    ObjectValidation,
    CommandBufferBuilding,
    CommandBufferSubmission,
    Presentation
  } renderStage;
};

struct RenderNode : public bt::Sequence {
  std::shared_ptr<RenderContext> renderContext;
};

struct ColorAttachment {};
struct DepthAttachment {};
struct InputAttachment {};
struct Pipeline {};
struct RenderPass {};
struct Subpass {};
struct CommandBuffer : public RenderNode {
  VkCommandBuffer cmd;
  VkCommandBufferLevel level;
  bool isRealized = false;

  Status update() override;

  void realize() {
    if (isRealized) return;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = renderContext->cmdPool;
    allocInfo.level = level;
    // TODO: logging
    vkAllocateCommandBuffers(renderContext->device, &allocInfo, &cmd);
    isRealized = true;
  }
};

// command buffer recording:
// validate buffers
// validate descriptors
// for each renderpass
// begin renderpass
// for each subpass:
// --bind pipeline
// --bind descriptor set(s)
// --bind vertex/index buffers
// --draw each object type's instances
// --next subpass

namespace vka {
class Device;
class Allocator;
class Image;
class RenderContext;

class Attachment {
public:
  VkExtent3D extent;
  VkAttachmentDescription description;

  void realize(
    std::shared_ptr<Device> device, std::shared_ptr<Allocator> allocator);

  void release();

private:
  std::shared_ptr<Image> image;
};
class Subpass {
public:
  void addInputAttachment(std::shared_ptr<Attachment> attachment);
  void addColorAttachment(std::shared_ptr<Attachment> attachment);
  void addPreserveAttachment(std::shared_ptr<Attachment> attachment);
  void setDepthStencilAttachment(std::shared_ptr<Attachment> attachment);

private:
  std::vector<std::shared_ptr<Attachment>> inputAttachments;
  std::vector<std::shared_ptr<Attachment>> colorAttachments;
  std::vector<std::shared_ptr<Attachment>> preserveAttachments;
  std::shared_ptr<Attachment> depthStencilAttachment;
};

class RenderPass {
  std::vector<std::shared_ptr<Subpass>> subpasses;
};

class FrameGraph {
  friend class Subpass;

public:
  std::shared_ptr<Attachment> addAttachment(
    VkFormat format, VkExtent3D size, VkSampleCountFlagBits sampleCount);
  std::shared_ptr<Subpass> addSubpass();
  void compile();

private:
  std::shared_ptr<Device> device;
  std::vector<std::shared_ptr<Attachment>> attachments;
  std::vector<std::shared_ptr<Subpass>> subpasses;
  std::vector<RenderPass> renderPasses;
};

void testUsage() {}
}  // namespace vka