#pragma once

#include <vulkan/vulkan.h>
#include <variant>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

namespace vka {
enum class device_features {
  robustBufferAccess,
  fullDrawIndexUint32,
  imageCubeArray,
  independentBlend,
  geometryShader,
  tessellationShader,
  sampleRateShading,
  dualSrcBlend,
  logicOp,
  multiDrawIndirect,
  drawIndirectFirstInstance,
  depthClamp,
  depthBiasClamp,
  fillModeNonSolid,
  depthBounds,
  wideLines,
  largePoints,
  alphaToOne,
  multiViewport,
  samplerAnisotropy,
  textureCompressionETC2,
  textureCompressionASTC_LDR,
  textureCompressionBC,
  occlusionQueryPrecise,
  pipelineStatisticsQuery,
  vertexPipelineStoresAndAtomics,
  fragmentStoresAndAtomics,
  shaderTessellationAndGeometryPointSize,
  shaderImageGatherExtended,
  shaderStorageImageExtendedFormats,
  shaderStorageImageMultisample,
  shaderStorageImageReadWithoutFormat,
  shaderStorageImageWriteWithoutFormat,
  shaderUniformBufferArrayDynamicIndexing,
  shaderSampledImageArrayDynamicIndexing,
  shaderStorageBufferArrayDynamicIndexing,
  shaderStorageImageArrayDynamicIndexing,
  shaderClipDistance,
  shaderCullDistance,
  shaderFloat64,
  shaderInt64,
  shaderInt16,
  shaderResourceResidency,
  shaderResourceMinLod,
  sparseBinding,
  sparseResidencyBuffer,
  sparseResidencyImage2D,
  sparseResidencyImage3D,
  sparseResidency2Samples,
  sparseResidency4Samples,
  sparseResidency8Samples,
  sparseResidency16Samples,
  sparseResidencyAliased,
  variableMultisampleRate,
  inheritedQueries
};

inline void to_vulkan_feature(
    VkPhysicalDeviceFeatures vulkanFeatures,
    device_features feature) {
  switch (feature) {
    case device_features::robustBufferAccess:
      vulkanFeatures.robustBufferAccess = VkBool32(true);
      break;
    case device_features::fullDrawIndexUint32:
      vulkanFeatures.fullDrawIndexUint32 = VkBool32(true);
      break;
    case device_features::imageCubeArray:
      vulkanFeatures.imageCubeArray = VkBool32(true);
      break;
    case device_features::independentBlend:
      vulkanFeatures.independentBlend = VkBool32(true);
      break;
    case device_features::geometryShader:
      vulkanFeatures.geometryShader = VkBool32(true);
      break;
    case device_features::tessellationShader:
      vulkanFeatures.tessellationShader = VkBool32(true);
      break;
    case device_features::sampleRateShading:
      vulkanFeatures.sampleRateShading = VkBool32(true);
      break;
    case device_features::dualSrcBlend:
      vulkanFeatures.dualSrcBlend = VkBool32(true);
      break;
    case device_features::logicOp:
      vulkanFeatures.logicOp = VkBool32(true);
      break;
    case device_features::multiDrawIndirect:
      vulkanFeatures.multiDrawIndirect = VkBool32(true);
      break;
    case device_features::drawIndirectFirstInstance:
      vulkanFeatures.drawIndirectFirstInstance = VkBool32(true);
      break;
    case device_features::depthClamp:
      vulkanFeatures.depthClamp = VkBool32(true);
      break;
    case device_features::depthBiasClamp:
      vulkanFeatures.depthBiasClamp = VkBool32(true);
      break;
    case device_features::fillModeNonSolid:
      vulkanFeatures.fillModeNonSolid = VkBool32(true);
      break;
    case device_features::depthBounds:
      vulkanFeatures.depthBounds = VkBool32(true);
      break;
    case device_features::wideLines:
      vulkanFeatures.wideLines = VkBool32(true);
      break;
    case device_features::largePoints:
      vulkanFeatures.largePoints = VkBool32(true);
      break;
    case device_features::alphaToOne:
      vulkanFeatures.alphaToOne = VkBool32(true);
      break;
    case device_features::multiViewport:
      vulkanFeatures.multiViewport = VkBool32(true);
      break;
    case device_features::samplerAnisotropy:
      vulkanFeatures.samplerAnisotropy = VkBool32(true);
      break;
    case device_features::textureCompressionETC2:
      vulkanFeatures.textureCompressionETC2 = VkBool32(true);
      break;
    case device_features::textureCompressionASTC_LDR:
      vulkanFeatures.textureCompressionASTC_LDR = VkBool32(true);
      break;
    case device_features::textureCompressionBC:
      vulkanFeatures.textureCompressionBC = VkBool32(true);
      break;
    case device_features::occlusionQueryPrecise:
      vulkanFeatures.occlusionQueryPrecise = VkBool32(true);
      break;
    case device_features::pipelineStatisticsQuery:
      vulkanFeatures.pipelineStatisticsQuery = VkBool32(true);
      break;
    case device_features::vertexPipelineStoresAndAtomics:
      vulkanFeatures.vertexPipelineStoresAndAtomics = VkBool32(true);
      break;
    case device_features::fragmentStoresAndAtomics:
      vulkanFeatures.fragmentStoresAndAtomics = VkBool32(true);
      break;
    case device_features::shaderTessellationAndGeometryPointSize:
      vulkanFeatures.shaderTessellationAndGeometryPointSize = VkBool32(true);
      break;
    case device_features::shaderImageGatherExtended:
      vulkanFeatures.shaderImageGatherExtended = VkBool32(true);
      break;
    case device_features::shaderStorageImageExtendedFormats:
      vulkanFeatures.shaderStorageImageExtendedFormats = VkBool32(true);
      break;
    case device_features::shaderStorageImageMultisample:
      vulkanFeatures.shaderStorageImageMultisample = VkBool32(true);
      break;
    case device_features::shaderStorageImageReadWithoutFormat:
      vulkanFeatures.shaderStorageImageReadWithoutFormat = VkBool32(true);
      break;
    case device_features::shaderStorageImageWriteWithoutFormat:
      vulkanFeatures.shaderStorageImageWriteWithoutFormat = VkBool32(true);
      break;
    case device_features::shaderUniformBufferArrayDynamicIndexing:
      vulkanFeatures.shaderUniformBufferArrayDynamicIndexing = VkBool32(true);
      break;
    case device_features::shaderSampledImageArrayDynamicIndexing:
      vulkanFeatures.shaderSampledImageArrayDynamicIndexing = VkBool32(true);
      break;
    case device_features::shaderStorageBufferArrayDynamicIndexing:
      vulkanFeatures.shaderStorageBufferArrayDynamicIndexing = VkBool32(true);
      break;
    case device_features::shaderStorageImageArrayDynamicIndexing:
      vulkanFeatures.shaderStorageImageArrayDynamicIndexing = VkBool32(true);
      break;
    case device_features::shaderClipDistance:
      vulkanFeatures.shaderClipDistance = VkBool32(true);
      break;
    case device_features::shaderCullDistance:
      vulkanFeatures.shaderCullDistance = VkBool32(true);
      break;
    case device_features::shaderFloat64:
      vulkanFeatures.shaderFloat64 = VkBool32(true);
      break;
    case device_features::shaderInt64:
      vulkanFeatures.shaderInt64 = VkBool32(true);
      break;
    case device_features::shaderInt16:
      vulkanFeatures.shaderInt16 = VkBool32(true);
      break;
    case device_features::shaderResourceResidency:
      vulkanFeatures.shaderResourceResidency = VkBool32(true);
      break;
    case device_features::shaderResourceMinLod:
      vulkanFeatures.shaderResourceMinLod = VkBool32(true);
      break;
    case device_features::sparseBinding:
      vulkanFeatures.sparseBinding = VkBool32(true);
      break;
    case device_features::sparseResidencyBuffer:
      vulkanFeatures.sparseResidencyBuffer = VkBool32(true);
      break;
    case device_features::sparseResidencyImage2D:
      vulkanFeatures.sparseResidencyImage2D = VkBool32(true);
      break;
    case device_features::sparseResidencyImage3D:
      vulkanFeatures.sparseResidencyImage3D = VkBool32(true);
      break;
    case device_features::sparseResidency2Samples:
      vulkanFeatures.sparseResidency2Samples = VkBool32(true);
      break;
    case device_features::sparseResidency4Samples:
      vulkanFeatures.sparseResidency4Samples = VkBool32(true);
      break;
    case device_features::sparseResidency8Samples:
      vulkanFeatures.sparseResidency8Samples = VkBool32(true);
      break;
    case device_features::sparseResidency16Samples:
      vulkanFeatures.sparseResidency16Samples = VkBool32(true);
      break;
    case device_features::sparseResidencyAliased:
      vulkanFeatures.sparseResidencyAliased = VkBool32(true);
      break;
    case device_features::variableMultisampleRate:
      vulkanFeatures.variableMultisampleRate = VkBool32(true);
      break;
    case device_features::inheritedQueries:
      vulkanFeatures.inheritedQueries = VkBool32(true);
      break;
  }
}

struct no_device_found {};
using physical_device_error = std::variant<no_device_found, VkResult>;

struct physical_device_selector {
  tl::expected<tl::optional<VkPhysicalDevice>, physical_device_error> select(VkInstance instance) {
    uint32_t count = {};
    auto countResult = vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (countResult != VK_SUCCESS) {
      return tl::make_unexpected(countResult);
    }
    std::vector<VkPhysicalDevice> physicalDevices = {};
    physicalDevices.resize(count);
    auto enumerateResult =
        vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());
    if (enumerateResult != VK_SUCCESS) {
      return tl::make_unexpected(enumerateResult);
    }
    std::vector<VkPhysicalDeviceProperties> properties = {};
    properties.resize(count);
    for (uint32_t i = {}; i < count; ++i) {
      vkGetPhysicalDeviceProperties(physicalDevices[i], &properties[i]);
    }

    if (m_deviceType) {
      for (uint32_t i = {}; i < count; ++i) {
        if (properties[i].deviceType == *m_deviceType) {
          return physicalDevices[i];
        }
      }
    }
    else if (count > 0) {
      return physicalDevices[0];
    }
    return tl::make_unexpected(no_device_found{});
  }

  physical_device_selector& device_type(VkPhysicalDeviceType deviceType) {
    m_deviceType = deviceType;
    return *this;
  }

private:
  tl::optional<VkPhysicalDeviceType> m_deviceType;
};

}  // namespace vka