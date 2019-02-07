#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>
#include <experimental/filesystem>
#include "io.hpp"
#include "move_into.hpp"
#include "logger.hpp"
#include <make_shader.hpp>
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


struct shader_module_data {
  std::unique_ptr<shader_module> shaderPtr;
  jshd::shader_data shaderData;
};

using shader_error = std::variant<VkResult, io::path_error>;
using shader_expected =
    tl::expected<shader_module_data, shader_error>;
inline auto make_shader(VkDevice device, std::string_view name) {
  auto j = json::parse(io::read_text_file(fs::path(name) + ".json"));
  auto
}
}  // namespace vka