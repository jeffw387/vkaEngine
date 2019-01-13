#include "Descriptors.hpp"

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
}  // namespace vka