#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <expected.hpp>

namespace vka {
class instance {
public:
  explicit instance(VkInstance instance) : m_instance(instance) {}
  ~instance() {
    vkDestroyInstance(m_instance, nullptr);
  }

  operator VkInstance() {
    return m_instance;
  }

private:
  VkInstance m_instance = {};
};

class instance_builder {
public:
  tl::expected<std::unique_ptr<instance>, VkResult> build() {
    m_create_info.pApplicationInfo = &m_app_info;
    m_create_info.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size());
    m_create_info.ppEnabledExtensionNames = m_extensions.data();
    m_create_info.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
    m_create_info.ppEnabledLayerNames = m_layers.data();
    VkInstance resultInstance = {};
    auto result = vkCreateInstance(&m_create_info, nullptr, &resultInstance);
    if(result != VK_SUCCESS) {
      return tl::unexpected<VkResult>(result);
    }
    return std::make_unique<instance>(resultInstance);
  }

  instance_builder& add_extension(const char* name) {
    m_extensions.push_back(name);
    return *this;
  }

  instance_builder& add_layer(const char* name) {
    m_layers.push_back(name);
    return *this;
  }

  instance_builder& set_engine_name(const char* name) {
    m_app_info.pEngineName = name;
    return *this;
  }

  instance_builder& set_app_name(const char* name) {
    m_app_info.pApplicationName = name;
    return *this;
  }

  instance_builder& set_api_version(int major, int minor, int patch) {
    m_app_info.apiVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  instance_builder& set_engine_version(int major, int minor, int patch) {
    m_app_info.engineVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;}
    
  instance_builder& set_app_version(int major, int minor, int patch) {
    m_app_info.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;}

private:
  std::vector<const char*> m_extensions = {};
  std::vector<const char*> m_layers = {};
  VkApplicationInfo m_app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  VkInstanceCreateInfo m_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
};
}