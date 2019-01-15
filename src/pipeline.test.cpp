#include "pipeline.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "descriptor_set_layout.hpp"
#include "pipeline_layout.hpp"
#include "shader_module.hpp"
#include "render_pass.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Create a default graphics pipeline") {
  platform::glfw::init();
  std::unique_ptr<instance> instancePtr = {};
  instance_builder{}
      .add_layer(standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) { REQUIRE(false); });

  VkPhysicalDevice physicalDevice = {};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { REQUIRE(false); });

  queue_family queueFamily = {};
  queue_family_builder{}
      .graphics_support()
      .queue(1.f)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<device> devicePtr = {};
  device_builder{}
      .add_queue_family(queueFamily)
      .physical_device(physicalDevice)
      .build(*instancePtr)
      .map(move_into{devicePtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<descriptor_set_layout> layoutPtr = {};
  descriptor_set_layout_builder{}
      .uniform_buffer(0, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{layoutPtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<pipeline_layout> pipelineLayoutPtr = {};
  pipeline_layout_builder{}
      .set_layout(*layoutPtr)
      .push_range(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4)
      .build(*devicePtr)
      .map(move_into{pipelineLayoutPtr})
      .map_error([](auto error) { REQUIRE(false); });
  
  std::unique_ptr<shader_module> vertShaderPtr = {};
  shader_module_builder{}
      .build(*devicePtr, "vert_shader.test.spv")
      .map(move_into{vertShaderPtr})
      .map_error([](auto error) { REQUIRE(false); });
  auto vertShaderStage = shader_stage_builder{}
    .shader_module(*vertShaderPtr, "main")
    .vertex()
    .build();

  std::unique_ptr<shader_module> fragShaderPtr = {};
  shader_module_builder{}
      .build(*devicePtr, "frag_shader.test.spv")
      .map(move_into{fragShaderPtr})
      .map_error([](auto error) { REQUIRE(false); });
  auto fragShaderStage = shader_stage_builder{}
    .shader_module(*fragShaderPtr, "main")
    .fragment()
    .build();
  
  std::unique_ptr<render_pass> renderPassPtr = {};
  render_pass_builder{}
    .add_attachment(attachment_builder{}
      .initial_layout(VK_IMAGE_LAYOUT_UNDEFINED)
      .final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      .format(VK_FORMAT_B8G8R8A8_UNORM)
      .samples(VK_SAMPLE_COUNT_1_BIT)
      .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
      .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
      .build())
    .add_subpass(subpass_builder{}
      .color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
      .build())
    .build(*devicePtr)
    .map(move_into{renderPassPtr})
    .map_error([](auto error) { REQUIRE(false); });
  
  std::unique_ptr<pipeline> graphicsPipelinePtr = {};
  graphics_pipeline_builder{}
    .render_pass(*renderPassPtr, 0)
    .pipeline_layout(*pipelineLayoutPtr)
    .polygon_mode(VkPolygonMode::VK_POLYGON_MODE_FILL)
    .primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .shader_stage(vertShaderStage)
    .shader_stage(fragShaderStage)
    .viewport_scissor({}, {})
    .color_attachment(no_blend_attachment{})
    .build(*devicePtr)
    .map(move_into{graphicsPipelinePtr})
    .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(graphicsPipelinePtr->operator VkPipeline() != VK_NULL_HANDLE);
  
}