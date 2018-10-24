#pragma once
#include <imgui.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <chrono>
#include "Device.hpp"

namespace vka {

struct GUIData {
  UniqueAllocatedImage fontImage;
  UniqueAllocatedBuffer vertexBuffer;
  size_t vertexByteOffset;
  size_t indexByteOffset;
  DescriptorSetLayout setLayout;
  DescriptorPool descriptorPool;
  DescriptorSet descriptorSet;
  PipelineLayout pipelineLayout;
  ShaderModule vertexShader;
  ShaderModule fragmentShader;
  GraphicsPipeline pipeline;
};
class GUI {
public:
  GUI(VkImage);
  ~GUI();
  ImGuiIO& getIO();
  void newFrame();
  ImDrawData* render();
};
}  // namespace vka