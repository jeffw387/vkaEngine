#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <tl/expected.hpp>

namespace vka {
struct semaphore {
  explicit semaphore(
      VkDevice device,
      VkSemaphore semaphoreHandle)
      : m_device(device), m_semaphore(semaphoreHandle) {}

  semaphore(const semaphore&) = delete;
  semaphore(semaphore&&) = default;
  semaphore& operator=(const semaphore&) = delete;
  semaphore& operator=(semaphore&&) = default;

  ~semaphore() noexcept {
    vkDestroySemaphore(m_device, m_semaphore, nullptr);
  }

  operator VkSemaphore() const noexcept {
    return m_semaphore;
  }

private:
  VkDevice m_device = {};
  VkSemaphore m_semaphore = {};
};

struct semaphore_builder {
  tl::expected<std::unique_ptr<semaphore>, VkResult> build(
      VkDevice device) {
    VkSemaphoreCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkSemaphore semaphoreHandle = {};
    auto result = vkCreateSemaphore(
        device, &createInfo, nullptr, &semaphoreHandle);
    if (result != VK_SUCCESS) {
      return tl::make_unexpected(result);
    }

    return std::make_unique<semaphore>(
        device, semaphoreHandle);
  }
};

}  // namespace vka