#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>
#include <experimental/filesystem>
#include "IO.hpp"
#include "move_into.hpp"
#include <spirv.hpp>
#include <spirv_cross.hpp>

namespace fs = std::experimental::filesystem;
namespace vka {


struct shader_module {
  shader_module(VkDevice device, VkShaderModule shaderModule, const uint32_t* dataPtr, size_t wordCount)
      : m_device(device), m_shaderModule(shaderModule), m_compiler{dataPtr, wordCount} {}
  shader_module(const shader_module&) = delete;
  shader_module(shader_module&&) = default;
  shader_module& operator=(const shader_module&) = delete;
  shader_module& operator=(shader_module&&) = default;
  ~shader_module() noexcept {
    vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
  }

  operator VkShaderModule() const noexcept { return m_shaderModule; }
  auto& compiler() { return m_compiler; }
private:
  VkDevice m_device{};
  VkShaderModule m_shaderModule{};
  spirv_cross::Compiler m_compiler;
};

constexpr auto setsFromShader = [](shader_module& shader) -> std::vector<VkDescriptorSetLayoutCreateInfo> {
  auto& comp = shader.compiler();
  auto resources = comp.get_shader_resources();
  auto specConstants = comp.get_specialization_constants();
  std::vector<VkDescriptorSetLayoutCreateInfo> layouts;
  std::unordered_map<uint32_t /*set*/, std::vector<VkDescriptorSetLayoutBinding>> bindings;
  for (spirv_cross::Resource& uniformBuffer : resources.uniform_buffers) {
    auto setID = comp.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
    auto binding = comp.get_decoration(resource.id, spv::Decoration::DecorationBinding);
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount =;
    bindings[setID].push_back(std::move(layoutBinding));
  }
  return {};
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

      return std::make_unique<shader_module>(device, shaderModule, shaderBytes32, shaderBytes.size() / 4);
    } else {
      return tl::make_unexpected(shaderBytesExpected.error());
    }
  }
};
}  // namespace vka