#pragma once
#include <imgui.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <chrono>
#include <memory>
#include "Image.hpp"
#include "Device.hpp"

namespace vka {

struct GUIData {
  const VkFormat fontFormat = VK_FORMAT_R8G8B8A8_UNORM;
  int width;
  int height;
  unsigned char* fontPixels;
  std::unique_ptr<Image> fontImage;
  std::unique_ptr<ImageView> fontImageView;
  std::unique_ptr<Sampler> fontSampler;
  std::unique_ptr<Buffer> indexBuffer;
  std::unique_ptr<Buffer> vertexBuffer;
  std::unique_ptr<DescriptorSetLayout> setLayout;
  std::unique_ptr<DescriptorPool> descriptorPool;
  std::unique_ptr<DescriptorSet> descriptorSet;
  std::unique_ptr<PipelineLayout> pipelineLayout;
  std::unique_ptr<ShaderModule> vertexShader;
  std::unique_ptr<ShaderModule> fragmentShader;
  std::unique_ptr<GraphicsPipeline> pipeline;
};
class GUI {
public:
  GUI();
  ~GUI();
};
}  // namespace vka