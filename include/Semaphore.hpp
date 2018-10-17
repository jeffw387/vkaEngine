#include <vulkan/vulkan.h>

namespace vka {
class Semaphore {
public:
  Semaphore() = default;
  Semaphore(VkDevice);
  Semaphore(Semaphore&&);
  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;
  Semaphore& operator=(Semaphore&&);
  operator VkSemaphore();
  ~Semaphore();
private:
  VkDevice device = VK_NULL_HANDLE;
  VkSemaphore semaphore = VK_NULL_HANDLE;
};
}