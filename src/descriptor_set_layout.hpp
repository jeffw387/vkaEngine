#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <tl/expected.hpp>
#include <make_fragment_shader.hpp>
#include <make_vertex_shader.hpp>
#include "shader_module.hpp"
#include "gsl-lite.hpp"
#include "sampler.hpp"

namespace vka {
struct descriptor_set_layout {
  explicit descriptor_set_layout(VkDevice device, VkDescriptorSetLayout layout)
      : m_device(device), m_layout(layout) {}
  descriptor_set_layout(const descriptor_set_layout&) = delete;
  descriptor_set_layout(descriptor_set_layout&&) = default;
  descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
  descriptor_set_layout& operator=(descriptor_set_layout&&) = default;
  ~descriptor_set_layout() {
    vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
  }
  operator VkDescriptorSetLayout() { return m_layout; }

private:
  VkDevice m_device = {};
  VkDescriptorSetLayout m_layout = {};
};

struct element_data {
  std::unique_ptr<sampler> samplerPtr;
};

struct binding_data {
  VkShaderStageFlags stageFlags;
  VkDescriptorType type;
  std::vector<element_data> elementData;
  bool immutableSamplers;
};

struct set_data {
  std::vector<binding_data> bindingData;
  std::unique_ptr<descriptor_set_layout> setLayoutPtr;
  uint32_t maxSets{1};
};

template <typename T>
inline void enlarge(T& v, size_t n) {
  if (v.size() < n) {
    v.resize(n);
  }
}

inline auto make_buffer_binding(
    VkShaderStageFlagBits stageBits,
    jshd::buffer_data bufferData) {
  binding_data bindingData{};
  auto& [stageFlags, type, elementData, immSamp] = bindingData;
  switch (bufferData.bufferType) {
    default:
    case jshd::buffer_type::uniform:
      if (bufferData.dynamic) {
        type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      } else {
        type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      }
      break;
    case jshd::buffer_type::storage:
      if (bufferData.dynamic) {
        type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
      } else {
        type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      }
      break;
  }
  elementData.resize(1);
  stageFlags |= stageBits;
  return bindingData;
}

inline auto make_image_binding(
    VkShaderStageFlagBits stageBits,
    jshd::image_data imageData) {
  binding_data bindingData{};
  auto& [stageFlags, type, elementData, immSamp] = bindingData;
  type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  elementData.resize(imageData.count);
  stageFlags |= stageBits;
  return bindingData;
}

inline auto make_sampler_binding(
    VkDevice device,
    VkShaderStageFlagBits stageBits,
    jshd::sampler_data samplerData) {
  binding_data bindingData{};
  auto& [stageFlags, type, elementData, immSamp] = bindingData;
  auto& [set, bindingNumber, samplerName, immutable, createInfos] = samplerData;
  if (immutable) {
    auto count = createInfos.size();
    elementData.resize(count);
    for (int i{}; i < count; ++i) {
      VkSampler vkSampler{};
      auto samplerResult =
          vkCreateSampler(device, &createInfos[i], nullptr, &vkSampler);
      if (samplerResult != VK_SUCCESS) {
        exit(samplerResult);
      }
      elementData[i].samplerPtr = std::make_unique<sampler>(device, vkSampler);
    }
  }
  return bindingData;
}

template <typename T>
constexpr auto get_shader_stage() -> VkShaderStageFlagBits {
  if constexpr (std::is_same_v<T, jshd::vertex_shader_data>) {
    return VK_SHADER_STAGE_VERTEX_BIT;
  } else if constexpr (std::is_same_v<T, jshd::fragment_shader_data>) {
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  } else {
    return VK_SHADER_STAGE_ALL;
  }
}

template <typename T>
auto parseShaderData = [](auto& setData, auto device, shader_data<T>& shaderModuleData) {
  auto& [ptr, shaderData] = shaderModuleData;
  auto shaderStage = get_shader_stage<decltype(shaderData)>();
  for (jshd::buffer_data bufferData : shaderData.buffers) {
    enlarge(setData, bufferData.set);
    auto& [bindingData, setLayoutPtr, m] = setData[bufferData.set];
    enlarge(bindingData, bufferData.binding);
    bindingData[bufferData.binding] =
        make_buffer_binding(shaderStage, bufferData);
  }
  for (jshd::image_data imageData : shaderData.images) {
    enlarge(setData, imageData.set);
    auto& [bindingData, setLayoutPtr, m] = setData[imageData.set];
    enlarge(bindingData, imageData.binding);
    bindingData[imageData.binding] =
        make_image_binding(shaderStage, imageData);
  }
  for (jshd::sampler_data samplerData : shaderData.samplers) {
    enlarge(setData, samplerData.set);
    auto& [bindingData, setLayoutPtr, m] = setData[samplerData.set];
    enlarge(bindingData, samplerData.binding);
    bindingData[samplerData.binding] =
        make_sampler_binding(device, shaderStage, samplerData);
  }
};

inline auto make_set_layouts(
    VkDevice device,
    shader_data<jshd::vertex_shader_data>& vertexShaderData,
    shader_data<jshd::fragment_shader_data>& fragmentShaderData) {
  std::vector<set_data> setData;

  parseShaderData<jshd::vertex_shader_data>(setData, device, vertexShaderData);
  parseShaderData<jshd::fragment_shader_data>(setData, device, fragmentShaderData);

  for (auto& set : setData) {
    auto bindingCount = static_cast<uint32_t>(set.bindingData.size());
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<std::vector<VkSampler>> bindingSamplers;
    bindings.reserve(bindingCount);
    bindingSamplers.resize(bindingCount);
    for (uint32_t i{}; i < bindingCount; ++i) {
      auto& [stage, type, elements, immutableSamplers] = set.bindingData[i];
      auto elementCount = static_cast<uint32_t>(elements.size());
      auto& samplers = bindingSamplers[i];
      if (immutableSamplers) {
        samplers.reserve(elementCount);
        std::for_each(
            std::begin(elements), std::end(elements), [&](auto& element) {
              samplers.push_back(*element.samplerPtr);
            });
        bindings.push_back({i, type, elementCount, stage, samplers.data()});
      } else {
        bindings.push_back({i, type, elementCount, stage, nullptr});
      }
    }
    VkDescriptorSetLayoutCreateInfo createInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    createInfo.bindingCount = bindingCount;
    createInfo.pBindings = bindings.data();
    VkDescriptorSetLayout setLayout{};
    auto layoutResult =
        vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout);
    if (layoutResult != VK_SUCCESS) {
      exit(layoutResult);
    }
    set.setLayoutPtr =
        std::make_unique<descriptor_set_layout>(device, setLayout);
  }
  return setData;
}
}  // namespace vka