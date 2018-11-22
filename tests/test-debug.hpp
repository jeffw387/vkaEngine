#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "ShaderModule.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"

struct DebugData {
  struct Vertex {
    glm::vec4 color;
    glm::vec2 pos;
  };
  std::shared_ptr<vka::Buffer> vertexBuffer;
  std::unique_ptr<vka::ShaderModule> vertexShader;
  std::unique_ptr<vka::ShaderModule> fragmentShader;
  std::shared_ptr<vka::PipelineLayout> pipelineLayout;
  std::shared_ptr<vka::GraphicsPipeline> pipeline;
};