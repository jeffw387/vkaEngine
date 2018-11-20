#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <variant>
#include <optional>
#include <map>
#include <memory>

namespace vka {
class DescriptorSetLayout;
class DescriptorSet;

struct DescriptorReference {
  VkDescriptorSet set;
  uint32_t bindingIndex;
  uint32_t arrayIndex;
};

class BufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo;
};

class StorageBufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo;
};

class DynamicBufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo;
};

class ImageDescriptor {
public:
  void operator()(VkImageView, VkImageLayout);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
};

class ImageSamplerDescriptor {
public:
  void operator()(VkImageView, VkImageLayout, VkSampler = VK_NULL_HANDLE);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
};

class SamplerDescriptor {
public:
  void operator()(VkSampler);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
};

using Descriptor = std::variant<
    BufferDescriptor,
    StorageBufferDescriptor,
    DynamicBufferDescriptor,
    ImageDescriptor,
    SamplerDescriptor,
    ImageSamplerDescriptor>;

class DescriptorSet {
public:
  DescriptorSet(VkDescriptorSet, DescriptorSetLayout*);
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
  std::map<uint32_t, std::vector<Descriptor>> bindings;
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
      DescriptorSetLayout* layout);
  void reset();

private:
  VkDevice device = VK_NULL_HANDLE;
  VkDescriptorPool poolHandle = VK_NULL_HANDLE;
};
}  // namespace vka