#pragma once
#include <vulkan/vulkan.h>
#include <tl/expected.hpp>
#include "queue_family.hpp"

namespace vka {
struct queue {
  queue() = default;
  explicit queue(VkQueue queueHandle) : m_queue(queueHandle) {}
  operator VkQueue() { return m_queue; }

private:
  VkQueue m_queue = {};
};

struct queue_index_out_of_bounds {};

struct queue_builder {
  tl::expected<queue, queue_index_out_of_bounds> build(VkDevice device) {
    if (m_queueIndex >= m_queueFamily.queuePriorities.size()) {
      return tl::make_unexpected(queue_index_out_of_bounds{});
    }
    VkQueue queueHandle = {};
    vkGetDeviceQueue(
        device, m_queueFamily.familyIndex, m_queueIndex, &queueHandle);
    return queue(queueHandle);
  }

  queue_builder& queue_info(queue_family family, uint32_t queueIndex) {
    m_queueFamily = family;
    m_queueIndex = queueIndex;
    return *this;
  }

private:
  queue_family m_queueFamily = {};
  uint32_t m_queueIndex = {};
};

}  // namespace vka