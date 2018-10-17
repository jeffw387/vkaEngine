#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <variant>
#include <optional>
#include <map>

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
  VkWriteDescriptorSet write;
};

class DynamicBufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo;
  VkWriteDescriptorSet write;
};

class ImageDescriptor {
public:
  void operator()(VkImageView, VkImageLayout);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
  VkWriteDescriptorSet write;
};

class ImageSamplerDescriptor {
public:
  void operator()(VkImageView, VkImageLayout, VkSampler = VK_NULL_HANDLE);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
  VkWriteDescriptorSet write;
};

class SamplerDescriptor {
public:
  void operator()(VkSampler);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo;
  VkWriteDescriptorSet write;
};

using Descriptor = std::variant<
    BufferDescriptor,
    DynamicBufferDescriptor,
    ImageDescriptor,
    SamplerDescriptor,
    ImageSamplerDescriptor>;

// need Descriptor& getDescriptor(uint32_t binding, uint32_t descriptor);

class DescriptorSet {
public:
  DescriptorSet() = default;
  DescriptorSet(VkDescriptorSet, DescriptorSetLayout*);
  DescriptorSet(DescriptorSet&&) = default;
  DescriptorSet& operator=(DescriptorSet&&) = default;
  DescriptorSet(const DescriptorSet&) = delete;
  DescriptorSet& operator=(const DescriptorSet&) = delete;
  ~DescriptorSet() = default;
  operator VkDescriptorSet();
  void validate(VkDevice device);
  template <typename T>
  T* getDescriptor(DescriptorReference ref) {
    auto& binding = bindings.at(ref.bindingIndex);
    auto& descriptor = binding.at(ref.arrayIndex);
    return &(std::get<T>(descriptor));
  }

private:
  VkDescriptorSet set;
  std::map<uint32_t, std::vector<Descriptor>> bindings;
};

class DescriptorPool {
public:
  DescriptorPool() = default;
  DescriptorPool(
      VkDevice device,
      std::vector<VkDescriptorPoolSize> poolSizes,
      uint32_t maxSets);
  DescriptorPool(DescriptorPool&&);
  DescriptorPool& operator=(DescriptorPool&&);
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;
  ~DescriptorPool();
  operator VkDescriptorPool() { return poolHandle; }

  std::vector<DescriptorSet> allocateDescriptorSets(
      std::vector<DescriptorSetLayout*> layouts);
  void reset();

private:
  VkDevice device = VK_NULL_HANDLE;
  VkDescriptorPool poolHandle = VK_NULL_HANDLE;
};
}  // namespace vka