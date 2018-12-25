#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <variant>
#include <optional>
#include <memory>
#include "Descriptors.hpp"

namespace vka {
class DescriptorSetLayout;

class DescriptorSet {
public:
  DescriptorSet(VkDescriptorSet, DescriptorBindings);
  operator VkDescriptorSet();
  void validate(VkDevice device);
  template <typename T>
  T* getDescriptor(DescriptorReference ref) {
    auto& binding = bindings.at(ref.bindingIndex);
    auto& descriptor = binding.at(ref.arrayIndex);
    return &(std::get<T>(descriptor));
  }
  void cmdExecuted() {}

private:
  VkDescriptorSet set;
  DescriptorBindings bindings;
};

class DescriptorPool {
public:
  DescriptorPool(
      VkDevice device,
      std::vector<VkDescriptorPoolSize> poolSizes,
      uint32_t maxSets);
  ~DescriptorPool();
  operator VkDescriptorPool() { return poolHandle; }

  std::shared_ptr<DescriptorSet> allocateDescriptorSet(
      DescriptorSetLayout* layout, DescriptorBindings);
  void reset();

private:
  VkDevice device = VK_NULL_HANDLE;
  VkDescriptorPool poolHandle = VK_NULL_HANDLE;
};
}  // namespace vka