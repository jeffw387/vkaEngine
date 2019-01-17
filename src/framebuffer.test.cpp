#include "framebuffer.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "memory_allocator.hpp"
#include "image.hpp"
#include "image_view.hpp"
#include "render_pass.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Create two images/views, create framebuffer and attach those views") {
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

  std::unique_ptr<allocator> allocatorPtr = {};
  allocator_builder{}
      .physical_device(physicalDevice)
      .device(*devicePtr)
      .build()
      .map(move_into{allocatorPtr})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<image> imagePtr0 = {};
  image_builder{}
      .gpu_only()
      .format(VK_FORMAT_R32G32B32A32_SFLOAT)
      .image_extent(100, 100)
      .transfer_destination()
      .sampled()
      .type_2d()
      .queue_family_index(queueFamily.familyIndex)
      .build(*allocatorPtr)
      .map(move_into{imagePtr0})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<image_view> viewPtr0 = {};
  image_view_builder{}
      .from_image(*imagePtr0)
      .build(*devicePtr)
      .map(move_into{viewPtr0})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<image> imagePtr1 = {};
  image_builder{}
      .gpu_only()
      .format(VK_FORMAT_R32G32B32A32_SFLOAT)
      .image_extent(100, 100)
      .transfer_destination()
      .sampled()
      .type_2d()
      .queue_family_index(queueFamily.familyIndex)
      .build(*allocatorPtr)
      .map(move_into{imagePtr1})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<image_view> viewPtr1 = {};
  image_view_builder{}
      .from_image(*imagePtr1)
      .build(*devicePtr)
      .map(move_into{viewPtr1})
      .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<render_pass> renderPassPtr = {};
  render_pass_builder{}
    .add_attachment(
      attachment_builder{}
        .initial_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        .final_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        .format(VK_FORMAT_R32G32B32A32_SFLOAT)
        .loadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
        .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
        .build())
    .add_attachment(
      attachment_builder{}
        .initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        .final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        .format(VK_FORMAT_R32G32B32A32_SFLOAT)
        .loadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
        .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
        .build())
    .add_subpass(
      subpass_builder{}
      .input_attachment(0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      .color_attachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
      .build())
    .build(*devicePtr)
    .map(move_into{renderPassPtr})
    .map_error([](auto error) { REQUIRE(false); });

  std::unique_ptr<framebuffer> framebufferPtr = {};
  framebuffer_builder{}
    .render_pass(*renderPassPtr)
    .dimensions(100, 100)
    .attachments({*viewPtr0, *viewPtr1})
    .build(*devicePtr)
    .map(move_into{framebufferPtr})
    .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(framebufferPtr->operator VkFramebuffer() != VK_NULL_HANDLE);
}