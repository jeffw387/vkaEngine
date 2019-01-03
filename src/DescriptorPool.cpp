#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

namespace vka {

DescriptorSet::DescriptorSet(VkDescriptorSet set, DescriptorSetLayout*)
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