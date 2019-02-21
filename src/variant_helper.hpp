#pragma once
#include <variant>

inline auto variant_size = [](auto v) -> size_t {
  return sizeof(v);
};