#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>
#include <variant>
#include <type_traits>

namespace vka {
struct sampler {
  explicit sampler(VkDevice device, VkSampler samplerHandle)
      : m_device(device), m_sampler(samplerHandle) {}

  sampler(const sampler&) = delete;
  sampler(sampler&&) = default;
  sampler& operator=(const sampler&) = delete;
  sampler& operator=(sampler&&) = default;

  ~sampler() noexcept {
    vkDestroySampler(m_device, m_sampler, nullptr);
  }

  operator VkSampler() const noexcept { return m_sampler; }

private:
  VkDevice m_device = {};
  VkSampler m_sampler = {};
};

using border_type = std::variant<float, int>;
struct black_border {};
struct white_border {};
struct transparent_border {};
using border_color = std::
    variant<black_border, white_border, transparent_border>;

template <class T>
struct always_false : std::false_type {};

inline auto border_visitor = [](auto&& color, auto&& type) {
  using colorT = std::decay_t<decltype(color)>;
  using typeT = std::decay_t<decltype(type)>;
  if constexpr (std::is_same_v<int, typeT>) {
    if constexpr (std::is_same_v<black_border, colorT>) {
      return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    } else if constexpr (std::is_same_v<
                             white_border,
                             colorT>) {
      return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    } else if constexpr (std::is_same_v<
                             transparent_border,
                             colorT>) {
      return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    } else {
      static_assert(
          always_false<colorT>::value,
          "Border color not supported!");
    }
  } else if constexpr (std::is_same_v<float, typeT>) {
    if constexpr (std::is_same_v<black_border, colorT>) {
      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    } else if constexpr (std::is_same_v<
                             white_border,
                             colorT>) {
      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    } else if constexpr (std::is_same_v<
                             transparent_border,
                             colorT>) {
      return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    } else {
      static_assert(
          always_false<colorT>::value,
          "Border color not supported!");
    }
  } else {
    static_assert(
        always_false<typeT>::value,
        "Border type not supported!");
  }
};

struct sampler_builder {
  tl::expected<std::unique_ptr<sampler>, VkResult> build(
      VkDevice device) {
    VkSamplerCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    createInfo.magFilter = m_magFilter;
    createInfo.minFilter = m_minFilter;
    createInfo.addressModeU = m_uAddressMode;
    createInfo.addressModeV = m_vAddressMode;
    createInfo.addressModeW = m_wAddressMode;
    createInfo.minLod = m_minLod;
    createInfo.maxLod = m_maxLod;
    createInfo.borderColor = std::visit(
        border_visitor, m_borderColor, m_borderType);
    createInfo.unnormalizedCoordinates =
        m_unnormalizedCoordinates;

    VkSampler samplerHandle = {};
    auto result = vkCreateSampler(
        device, &createInfo, nullptr, &samplerHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<sampler>(device, samplerHandle);
  }

  sampler_builder& filter_types(
      VkFilter magFilter,
      VkFilter minFilter) {
    m_magFilter = magFilter;
    m_minFilter = minFilter;
    return *this;
  }

  sampler_builder& address_modes(
      VkSamplerAddressMode u,
      VkSamplerAddressMode v,
      VkSamplerAddressMode w) {
    m_uAddressMode = u;
    m_vAddressMode = v;
    m_wAddressMode = w;
    return *this;
  }

  sampler_builder& lod_limits(float minLod, float maxLod) {
    m_minLod = minLod;
    m_maxLod = maxLod;
    return *this;
  }

  sampler_builder& set_border_color(border_color color) {
    m_borderColor = color;
    return *this;
  }

  sampler_builder& set_border_type(border_type type) {
    m_borderType = type;
    return *this;
  }

  sampler_builder& unnormalized_coordinates() {
    m_unnormalizedCoordinates = true;
    return *this;
  }

private:
  VkFilter m_magFilter = VkFilter::VK_FILTER_NEAREST;
  VkFilter m_minFilter = VkFilter::VK_FILTER_NEAREST;
  VkSamplerAddressMode m_uAddressMode = {};
  VkSamplerAddressMode m_vAddressMode = {};
  VkSamplerAddressMode m_wAddressMode = {};
  float m_minLod = {};
  float m_maxLod = {};
  border_color m_borderColor = transparent_border{};
  border_type m_borderType = float{};
  VkBool32 m_unnormalizedCoordinates = {};
};
}  // namespace vka