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

using shader_error = std::variant<VkResult, IO::path_error>;
using shader_expected =
    tl::expected<std::unique_ptr<shader_module>, shader_error>;
struct shader_module_builder {
  auto build(VkDevice device, fs::path shaderPath) -> shader_expected {
    if (auto shaderBytesExpected = IO::loadBinaryFile(shaderPath)) {
      auto& shaderBytes = shaderBytesExpected.value();
      auto shaderBytes32 = reinterpret_cast<const uint32_t*>(shaderBytes.data());

      VkShaderModuleCreateInfo createInfo{
          VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
      createInfo.codeSize = shaderBytes.size();
      createInfo.pCode = shaderBytes32;

      VkShaderModule shaderModule = {};
      auto shaderResult =
          vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
      if (shaderResult != VK_SUCCESS) {
        return tl::make_unexpected(shaderResult);
      }

      return std::make_unique<shader_module>(device, shaderModule);
    } else {
      return tl::make_unexpected(shaderBytesExpected.error());
    }
  }
};
}  // namespace vka