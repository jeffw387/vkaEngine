#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <experimental/filesystem>
#include <tl/expected.hpp>
#include <string>
#include "gsl-lite.hpp"

namespace io {
namespace fs = std::experimental::filesystem;

enum class path_error {
  Success = 0,
  PathProblem,
  ReadProblem,
  WriteProblem,
  UnknownProblem
};

inline tl::expected<std::vector<uint8_t>, path_error> read_binary_file(
    fs::path filePath) {
  if (!fs::exists(filePath)) {
    return tl::make_unexpected(path_error::PathProblem);
  }
  std::fstream fileStream(
      filePath, std::ios_base::binary | std::ios_base::in);
  size_t streamLength{};
  if (fileStream) {
    std::basic_stringstream<uint8_t> ss;
    ss << fileStream.rdbuf();
    if (!fileStream) {
      return tl::make_unexpected(path_error::ReadProblem);
    }
    return ss.str();
  }
  return tl::make_unexpected(path_error::UnknownProblem);
}

template <typename T>
inline tl::expected<void, path_error> write_binary_file(
    fs::path filePath,
    gsl::span<T> data) {
  std::ofstream fileStream(
      filePath, std::ios_base::out | std::ios_base::binary);
  if (fileStream) {
    fileStream.seekp(0, std::ios::beg);
    auto writeSize = data.length_bytes();
    fileStream.write((char*)data.data(), writeSize);
    if (fileStream.bad() || fileStream.fail()) {
      return tl::make_unexpected(path_error::WriteProblem);
    }
    return {};
  }
  return tl::make_unexpected(path_error::PathProblem);
}

inline auto read_text_file(fs::path filePath) {
  std::ifstream f(filePath);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}
}  // namespace IO