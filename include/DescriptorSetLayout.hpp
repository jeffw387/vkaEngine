#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>
#include <vector>

namespace vka {
class Device;

class DescriptorSetLayout {
public:
  DescriptorSetLayout() = default;
  DescriptorSetLayout(
      VkDevice device,
      std::vector<VkDescriptorSetLayoutBinding>);
  DescriptorSetLayout(DescriptorSetLayout&&);
  DescriptorSetLayout& operator=(DescriptorSetLayout&&);
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
  ~DescriptorSetLayout();

  operator VkDescriptorSetLayout() { return layoutHandle; }
  const std::vector<VkDescriptorSetLayoutBinding>& getBindings() {
    return bindings;
  }

private:
  VkDevice device;
  VkDescriptorSetLayout layoutHandle;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};
}  // namespace vka