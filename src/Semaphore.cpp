#include "Semaphore.hpp"
#include <utility>

namespace vka {
Semaphore::Semaphore(VkDevice device) : device(device) {
  VkSemaphoreCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);
}

Semaphore& Semaphore::operator=(Semaphore&& other) {
  if (this != &other) {
    device = other.device;
    semaphore = other.semaphore;
    other.device = {};
    other.semaphore = {};
  }
  return *this;
}

Semaphore::Semaphore(Semaphore&& other) { *this = std::move(other); }

Semaphore::operator VkSemaphore() { return semaphore; }

Semaphore::~Semaphore() {
  if (device != VK_NULL_HANDLE && semaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, semaphore, nullptr);
  }
}
}  // namespace vka