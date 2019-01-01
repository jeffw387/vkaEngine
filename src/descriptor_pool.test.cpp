#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "descriptor_pool.hpp"
#include "GLFW.hpp"
#include "Instance.hpp"
#include "Device.hpp"

TEST_CASE("Create descriptor pool") {
  using namespace platform;
  GLFW::init();
  auto instance = std::make_unique<vka::Instance<GLFW>>(
      vka::InstanceCreateInfo{"app name", Version{0, 0, 1}, {}, {}});
  auto surface = instance->createSurface(vka::SurfaceCreateInfo{500, 500, "window title"});
  auto device = instance->createDevice(
    surface.get(), 
    {}, 
    {}, 
    [](auto deviceInfo){ return deviceInfo.physicalDevices[0]; });
  
  auto pool_result = vka::descriptor_pool_builder{}
    .max_sets(1)
    .add_type(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1})
    .build(*device);
  REQUIRE(std::get<VkResult>(pool_result) == VK_SUCCESS);
}