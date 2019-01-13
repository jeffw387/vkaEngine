#pragma once
#include <optional>
#include <vector>
#include <map>
#include <variant>
#include <vulkan/vulkan.h>

namespace vka {
struct DescriptorReference {
  VkDescriptorSet set;
  uint32_t bindingIndex;
  uint32_t arrayIndex;
};

enum class DescriptorType {
  Uniform = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  DynamicUniform = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  Storage = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
  // DynamicStorage
  Image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
  ImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
  Sampler = VK_DESCRIPTOR_TYPE_SAMPLER
};

class BufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::Uniform;
  }

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo = {};
};

class StorageBufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::Storage;
  }
  VkShaderStageFlags shaderStage = {};

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo = {};
};

class DynamicBufferDescriptor {
public:
  void operator()(VkBuffer newBuffer, VkDeviceSize newRange);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::DynamicUniform;
  }
  VkShaderStageFlags shaderStage = {};

private:
  bool valid = false;
  VkDescriptorBufferInfo bufferInfo = {};
};

class ImageDescriptor {
public:
  void operator()(VkImageView, VkImageLayout);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::Image;
  }
  VkShaderStageFlags shaderStage = {};

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo = {};
};

class ImageSamplerDescriptor {
public:
  void operator()(VkImageView, VkImageLayout, VkSampler = VK_NULL_HANDLE);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::ImageSampler;
  }
  VkShaderStageFlags shaderStage = {};

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo = {};
};

class SamplerDescriptor {
public:
  void operator()(VkSampler);
  std::optional<VkWriteDescriptorSet> writeDescriptor(DescriptorReference);
  static constexpr DescriptorType descriptorType() {
    return DescriptorType::Sampler;
  }
  VkShaderStageFlags shaderStage = {};

private:
  bool valid = false;
  VkDescriptorImageInfo imageInfo = {};
};

using Descriptor = std::variant<
    BufferDescriptor,
    StorageBufferDescriptor,
    DynamicBufferDescriptor,
    ImageDescriptor,
    SamplerDescriptor,
    ImageSamplerDescriptor>;

struct DescriptorBinding {
  uint32_t binding = {};
  VkShaderStageFlags stageFlags = {};
};
using DescriptorBindings =
    std::map<DescriptorBinding, std::vector<Descriptor*>>;

}  // namespace vka