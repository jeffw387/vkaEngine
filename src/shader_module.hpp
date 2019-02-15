#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include <experimental/filesystem>
#include "io.hpp"
#include "move_into.hpp"
#include "logger.hpp"
#include <make_fragment_shader.hpp>
#include <make_vertex_shader.hpp>
#include <nlohmann/json.hpp>

using nlohmann::json;
namespace fs = std::experimental::filesystem;
namespace vka {

struct shader_module {
  shader_module(VkDevice device, VkShaderModule shaderModule)
      : m_device(device), m_shaderModule(shaderModule) {}
  shader_module(const shader_module&) = delete;
  shader_module(shader_module&&) = default;
  shader_module& operator=(const shader_module&) = delete;
  shader_module& operator=(shader_module&&) = default;
  ~shader_module() noexcept {
    vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
  }

  operator VkShaderModule() const noexcept { return m_shaderModule; }

private:
  VkDevice m_device{};
  VkShaderModule m_shaderModule{};
};

template <typename T>
struct shader_data {
  std::unique_ptr<shader_module> shaderPtr;
  T shaderData;
};

using shader_error = std::variant<VkResult, io::path_error>;

template <typename T>
using shader_expected = tl::expected<shader_data<T>, shader_error>;

template <typename T>
inline auto make_shader(VkDevice device, std::string_view name)
    -> shader_expected<T> {
  shader_data<T> result{};
  auto j = json::parse(io::read_text_file(std::string(name) + ".json"));
  if constexpr (std::is_same_v<T, jshd::vertex_shader_data>) {
    result.shaderData = jshd::vertex_shader_deserialize(j);
  } else if constexpr (std::is_same_v<T, jshd::fragment_shader_data>) {
    result.shaderData = jshd::fragment_shader_deserialize(j);
  }
  if (auto b = io::read_binary_file(fs::path(name.data()))) {
    VkShaderModuleCreateInfo createInfo{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = static_cast<uint32_t>(b->size());
    createInfo.pCode = reinterpret_cast<uint32_t*>(b->data());
    VkShaderModule shaderModule{};
    auto shaderResult =
        vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (shaderResult != VK_SUCCESS) {
      return tl::make_unexpected(shaderResult);
    }
    result.shaderPtr = std::make_unique<shader_module>(device, shaderModule);
  } else {
    return tl::make_unexpected(b.error());
  }
  return result;
}
}  // namespace vka