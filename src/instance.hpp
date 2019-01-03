#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

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
  auto build() {
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    create_info.ppEnabledLayerNames = layers.data();
    VkInstance result_instance = {};
    auto result = vkCreateInstance(&create_info, nullptr, &result_instance);
    auto instance_unique = std::make_unique<instance>(result_instance);
    return std::make_pair(std::move(instance_unique), std::move(result));
  }

  instance_builder& add_extension(const char* name) {
    extensions.push_back(name);
    return *this;
  }

  instance_builder& add_layer(const char* name) {
    layers.push_back(name);
    return *this;
  }

  instance_builder& set_engine_name(const char* name) {
    app_info.pEngineName = name;
    return *this;
  }

  instance_builder& set_app_name(const char* name) {
    app_info.pApplicationName = name;
    return *this;
  }

  instance_builder& set_api_version(int major, int minor, int patch) {
    app_info.apiVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  instance_builder& set_engine_version(int major, int minor, int patch) {
    app_info.engineVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;}
    
  instance_builder& set_app_version(int major, int minor, int patch) {
    app_info.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;}

private:
  std::vector<const char*> extensions;
  std::vector<const char*> layers;
  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
};
}