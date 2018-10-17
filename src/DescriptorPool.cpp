#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include <GLFW/glfw3.h>
#include <vector>

namespace vka {

std::optional<VkWriteDescriptorSet> BufferDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.descriptorCount = 1;
  write.pBufferInfo = &bufferInfo;
  result = std::move(write);
  return result;
}

std::optional<VkWriteDescriptorSet> DynamicBufferDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  write.descriptorCount = 1;
  write.pBufferInfo = &bufferInfo;
  result = std::move(write);
  return result;
}

std::optional<VkWriteDescriptorSet> ImageDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  write.descriptorCount = 1;
  write.pImageInfo = &imageInfo;
  result = std::move(write);
  return result;
}

std::optional<VkWriteDescriptorSet> SamplerDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  write.descriptorCount = 1;
  write.pImageInfo = &imageInfo;
  result = std::move(write);
  return result;
}

std::optional<VkWriteDescriptorSet> ImageSamplerDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.pImageInfo = &imageInfo;
  result = std::move(write);
  return result;
}

void BufferDescriptor::operator()(VkBuffer newBuffer, VkDeviceSize newRange) {
  bufferInfo.buffer = newBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = newRange;
  valid = false;
}

void DynamicBufferDescriptor::operator()(
    VkBuffer newBuffer,
    VkDeviceSize newRange) {
  bufferInfo.buffer = newBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = newRange;
  valid = false;
}

void ImageDescriptor::operator()(VkImageView view, VkImageLayout layout) {
  imageInfo.imageView = view;
  imageInfo.imageLayout = layout;
  valid = false;
}

void SamplerDescriptor::operator()(VkSampler sampler) {
  imageInfo.sampler = sampler;
  valid = false;
}

void ImageSamplerDescriptor::
operator()(VkImageView view, VkImageLayout layout, VkSampler sampler) {
  imageInfo.imageView = view;
  imageInfo.imageLayout = layout;
  imageInfo.sampler = sampler;
  valid = false;
}

DescriptorSet::DescriptorSet(VkDescriptorSet set, DescriptorSetLayout* layout)
    : set(set) {
  const auto& layoutBindings = layout->getBindings();
  for (const auto& binding : layoutBindings) {
    auto addDescriptors = [&](Descriptor descriptor) {
      for (auto descriptorIndex = 0U; descriptorIndex < binding.descriptorCount;
           ++descriptorIndex) {
        bindings[binding.binding].push_back(descriptor);
      }
    };

    switch (binding.descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        addDescriptors(BufferDescriptor());
        break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        addDescriptors(DynamicBufferDescriptor());
        break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        addDescriptors(ImageSamplerDescriptor());
        break;
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        addDescriptors(ImageDescriptor());
        break;
      case VK_DESCRIPTOR_TYPE_SAMPLER:
        addDescriptors(SamplerDescriptor());
        break;
      default:
        // TODO: error handling here;
        break;
    }
  }
}

void DescriptorSet::validate(VkDevice device) {
  std::vector<VkWriteDescriptorSet> writes;
  for (auto& [bindIndex, binding] : bindings) {
    uint32_t arrayIndex{};
    for (Descriptor& descriptor : binding) {
      auto write = std::visit(
          [bindIndex = bindIndex, set = set, arrayIndex = arrayIndex](
              auto& descriptorVariant) {
            return descriptorVariant.writeDescriptor(
                {set, bindIndex, arrayIndex});
          },
          descriptor);
      if (write) {
        writes.push_back(*std::move(write));
      }
      ++arrayIndex;
    }
  }
  vkUpdateDescriptorSets(
      device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

DescriptorSet::operator VkDescriptorSet() { return set; }

DescriptorPool::DescriptorPool(
    VkDevice device,
    std::vector<VkDescriptorPoolSize> poolSizes,
    uint32_t maxSets)
    : device(device) {
  VkDescriptorPoolCreateInfo createInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();
  createInfo.maxSets = maxSets;
  vkCreateDescriptorPool(device, &createInfo, nullptr, &poolHandle);
}

std::vector<DescriptorSet> DescriptorPool::allocateDescriptorSets(
    std::vector<DescriptorSetLayout*> layouts) {
  std::vector<VkDescriptorSetLayout> vkLayouts;
  std::vector<VkDescriptorSet> vkSets;
  std::vector<DescriptorSet> sets;

  vkSets.resize(layouts.size());
  sets.resize(layouts.size());
  for (auto i = 0U; i < layouts.size(); ++i) {
    VkDescriptorSetLayout vkLayout = *layouts[i];
    vkLayouts.push_back(vkLayout);
  }
  VkDescriptorSetAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool = poolHandle;
  allocateInfo.descriptorSetCount = static_cast<uint32_t>(vkLayouts.size());
  allocateInfo.pSetLayouts = vkLayouts.data();
  vkAllocateDescriptorSets(device, &allocateInfo, vkSets.data());

  for (auto i = 0U; i < sets.size(); ++i) {
    sets[i] = DescriptorSet(vkSets[i], layouts[i]);
  }
  return std::move(sets);
}

void DescriptorPool::reset() { vkResetDescriptorPool(device, poolHandle, 0); }

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) {
  if (this != &other) {
    device = other.device;
    poolHandle = other.poolHandle;
    other.device = {};
    other.poolHandle = {};
  }
  return *this;
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) {
  *this = std::move(other);
}

DescriptorPool::~DescriptorPool() {
  if (device != VK_NULL_HANDLE && poolHandle != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, poolHandle, nullptr);
  }
}
}  // namespace vka