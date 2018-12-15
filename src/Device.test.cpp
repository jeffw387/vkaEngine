#include "GLFW.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Create device-dependent objects") {
  auto instance =
      std::make_unique<vka::Instance<GLFW::Context>>(vka::InstanceCreateInfo{
          "test app name",
          Version{0, 0, 0},
          std::vector<const char*>{},
          std::vector<const char*>{"VK_LAYER_LUNARG_standard_validation",
                                   "VK_LAYER_LUNARG_api_dump"}});
  auto surface = instance->createSurface(
      vka::SurfaceCreateInfo{200, 200, "test window title"});
  auto device = instance->createDevice(
      surface.get(),
      std::vector<const char*>{},
      std::vector<vka::PhysicalDeviceFeatures>{},
      [](auto deviceInfo) { return deviceInfo.physicalDevices.at(0); });

  auto buffer = device->createBuffer(
      1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, true);

  auto image = device->createImage2D(
      VkExtent2D{200, 200},
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_USAGE_SAMPLED_BIT,
      vka::ImageAspect::Color,
      true);

  auto swapchain = device->createSwapchain();

  auto createInfo = vka::RenderPassCreateInfo{};
  auto pass0 = createInfo.addGraphicsSubpass();
  auto renderPass = device->createRenderPass(createInfo);

  auto cache = device->createPipelineCache();

  auto layout = device->createPipelineLayout({}, {});

  auto pipelineCreateInfo =
      vka::GraphicsPipelineCreateInfo{*layout, *renderPass, 0};
  auto pipeline = device->createGraphicsPipeline(*cache, pipelineCreateInfo);

  SECTION("Create buffer") {
    REQUIRE(buffer->operator VkBuffer() != VK_NULL_HANDLE);
    REQUIRE(buffer->operator VmaAllocation() != nullptr);
  }

  SECTION("Create Image2D") {
    REQUIRE(image->operator VkImage() != VK_NULL_HANDLE);
    REQUIRE(image->operator VmaAllocation() != nullptr);
  }
  SECTION("Create Swapchain") {
    REQUIRE(swapchain->operator VkSwapchainKHR() != VK_NULL_HANDLE);
  }
  SECTION("Create Render Pass") {
    REQUIRE(renderPass->operator VkRenderPass() != VK_NULL_HANDLE);
  }
  SECTION("Create Pipeline Cache") {
    REQUIRE(cache->operator VkPipelineCache() != VK_NULL_HANDLE);
  }
  SECTION("Create Pipeline Layout") {
    REQUIRE(layout->operator VkPipelineLayout() != VK_NULL_HANDLE);
  }
  SECTION("Create Graphics Pipeline") {
    REQUIRE(pipeline->operator VkPipeline() != VK_NULL_HANDLE);
  }
}