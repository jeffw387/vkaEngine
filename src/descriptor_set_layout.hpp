#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <tl/expected.hpp>
#include "gsl-lite.hpp"

namespace vka {
struct descriptor_set_layout {
  explicit descriptor_set_layout(
      VkDevice device,
      VkDescriptorSetLayout layout,
      std::vector<VkDescriptorSetLayoutBinding> bindings)
      : m_device(device), m_layout(layout), m_bindings(std::move(bindings)) {}
  descriptor_set_layout(const descriptor_set_layout&) = delete;
  descriptor_set_layout(descriptor_set_layout&&) = default;
  descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
  descriptor_set_layout& operator=(descriptor_set_layout&&) = default;
  ~descriptor_set_layout() {
    vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
  }
  operator VkDescriptorSetLayout() { return m_layout; }
  const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings() const
      noexcept {
    return m_bindings;
  }

private:
  VkDevice m_device = {};
  VkDescriptorSetLayout m_layout = {};
  std::vector<VkDescriptorSetLayoutBinding> m_bindings = {};
};

struct descriptor_set_layout_builder {
  tl::expected<std::unique_ptr<descriptor_set_layout>, VkResult> build(
      VkDevice device) {
    VkDescriptorSetLayoutCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    createInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
    createInfo.pBindings = m_bindings.data();

    VkDescriptorSetLayout layout = {};
    auto result =
        vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }
    return std::make_unique<descriptor_set_layout>(device, layout, m_bindings);
  }

  descriptor_set_layout_builder& uniform_buffer(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& uniform_buffer_dynamic(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& storage_buffer(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& storage_buffer_dynamic(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder&
  image(uint32_t binding, uint32_t count, VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& sampler(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags,
      gsl::span<VkSampler> samplers) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;
    if (samplers.size() > 0) {
      bindingDescription.pImmutableSamplers = samplers.data();
    }

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& combined_image_sampler(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags,
      gsl::span<VkSampler> samplers) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;
    if (samplers.size() > 0) {
      bindingDescription.pImmutableSamplers = samplers.data();
    }

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

  descriptor_set_layout_builder& input_attachment(
      uint32_t binding,
      uint32_t count,
      VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding bindingDescription = {};
    bindingDescription.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindingDescription.descriptorCount = count;
    bindingDescription.binding = binding;
    bindingDescription.stageFlags = stageFlags;

    m_bindings.push_back(std::move(bindingDescription));
    return *this;
  }

private:
  std::vector<VkDescriptorSetLayoutBinding> m_bindings = {};
};
}  // namespace vka