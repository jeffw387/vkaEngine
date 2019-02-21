#include "render_pass.hpp"

#include <catch2/catch.hpp>
#include "platform_glfw.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "queue_family.hpp"
#include "device.hpp"
#include "move_into.hpp"

using namespace vka;
TEST_CASE("Create a render pass with a single subpass") {
  platform::glfw::init();
  std::unique_ptr<vka::instance> instancePtr = {};
  vka::instance_builder{}
      .add_layer(vka::standard_validation)
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

  subpass mainSubpass =
      subpass_builder{}
          .color_attachment(
              0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
          .depth_attachment(
              1,
              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
          .build();
  std::unique_ptr<render_pass> renderPassPtr = {};
  render_pass_builder{}
      .add_attachment(
          attachment_builder{}
              .format(VK_FORMAT_B8G8R8A8_UNORM)
              .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
              .initial_layout(VK_IMAGE_LAYOUT_UNDEFINED)
              .final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
              .build())
      .add_attachment(
          attachment_builder{}
              .format(VK_FORMAT_D32_SFLOAT)
              .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .storeOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
              .initial_layout(VK_IMAGE_LAYOUT_UNDEFINED)
              .final_layout(VK_IMAGE_LAYOUT_UNDEFINED)
              .build())
      .add_subpass(mainSubpass)
      .build(*devicePtr)
      .map(move_into{renderPassPtr})
      .map_error([](auto error) { REQUIRE(false); });
  REQUIRE(
      renderPassPtr->operator VkRenderPass() !=
      VK_NULL_HANDLE);
}