#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Image.hpp"

namespace vka {
class Framebuffer {
  friend class CommandBuffer;

public:
  Framebuffer(
      VkDevice,
      VkRenderPass,
      std::vector<std::shared_ptr<ImageView>>,
      VkExtent2D);
  ~Framebuffer();
  operator VkFramebuffer() const noexcept;
  void cmdExecuted() {}

private:
  VkDevice device = {};
  std::vector<std::shared_ptr<ImageView>> views;
  VkFramebuffer framebuffer = {};
};
}  // namespace vka