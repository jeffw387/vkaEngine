#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include <GLFW/glfw3.h>
#include <vector>

namespace vka {

std::optional<VkWriteDescriptorSet> BufferDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid || bufferInfo.range == 0) {
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
  valid = true;
  return result;
}

std::optional<VkWriteDescriptorSet> StorageBufferDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid || bufferInfo.range == 0) {
    return result;
  }
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = ref.set;
  write.dstBinding = ref.bindingIndex;
  write.dstArrayElement = ref.arrayIndex;
  write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  write.descriptorCount = 1;
  write.pBufferInfo = &bufferInfo;
  result = std::move(write);
  valid = true;
  return result;
}

std::optional<VkWriteDescriptorSet> DynamicBufferDescriptor::writeDescriptor(
    DescriptorReference ref) {
  std::optional<VkWriteDescriptorSet> result;
  if (valid || bufferInfo.range == 0) {
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
  valid = true;
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
  valid = true;
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
  valid = true;
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
  valid = true;
  return result;
}

void BufferDescriptor::operator()(VkBuffer newBuffer, VkDeviceSize newRange) {
  bufferInfo.buffer = newBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = newRange;
  valid = false;
}

void StorageBufferDescriptor::operator()(
    VkBuffer newBuffer,
    VkDeviceSize newRange) {
  bufferInfo.buffer = newBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = VK_WHOLE_SIZE;
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
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        addDescriptors(StorageBufferDescriptor());
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
        throw std::runtime_error("Descriptor type not implemented!");
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

std::shared_ptr<DescriptorSet> DescriptorPool::allocateDescriptorSet(
    DescriptorSetLayout* layout) {
  VkDescriptorSetLayout vkLayout = *layout;
  VkDescriptorSet vkSet;

  VkDescriptorSetAllocateInfo allocateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool = poolHandle;
  allocateInfo.descriptorSetCount = 1;
  allocateInfo.pSetLayouts = &vkLayout;
  vkAllocateDescriptorSets(device, &allocateInfo, &vkSet);

  return std::make_shared<DescriptorSet>(vkSet, layout);
}

void DescriptorPool::reset() { vkResetDescriptorPool(device, poolHandle, 0); }

DescriptorPool::~DescriptorPool() {
  if (poolHandle != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, poolHandle, nullptr);
  }
}
}  // namespace vka