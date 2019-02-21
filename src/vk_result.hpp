#pragma once
#include <vulkan/vulkan.h>
#include <system_error>
namespace std {
template <>
struct is_error_code_enum<VkResult> : std::true_type {};
}  // namespace std

namespace detail {
class VkResult_category : public std::error_category {
public:
  virtual const char* name() const noexcept override final {
    return "VkResult";
  }
  virtual std::string message(
      int result) const override final {
    return "unknown VkResult";
  }
};
}  // namespace detail

static const detail::VkResult_category&
VkResult_category() {
  static detail::VkResult_category c;
  return c;
}

static std::error_code make_error_code(VkResult r) {
  return {static_cast<int>(r), VkResult_category()};
}