#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Image.hpp"

namespace vka {
class Framebuffer {
public:
  Framebuffer(VkDevice,
    VkRenderPass,
    std::vector<std::shared_ptr<ImageView>>,
    VkExtent2D);
  ~Framebuffer();
  operator VkFramebuffer() const noexcept;
private:
  VkDevice device = {};
  std::vector<std::shared_ptr<ImageView>> views;
  VkFramebuffer framebuffer = {};
};
}