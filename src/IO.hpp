#pragma once
#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include <tl/expected.hpp>
#include <string>
#include "gsl-lite.hpp"

namespace IO {
namespace fs = std::experimental::filesystem;

enum class path_error {
  Success = 0,
  PathProblem,
  ReadProblem,
  WriteProblem,
  UnknownProblem
};

inline tl::expected<std::vector<uint8_t>, path_error> loadBinaryFile(
    fs::path filePath) {
  if (!fs::exists(filePath)) {
    return tl::make_unexpected(path_error::PathProblem);
  }
  std::ifstream fileStream(
      filePath, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
  size_t streamLength{};
  std::vector<uint8_t> result;
  if (fileStream) {
    streamLength = fileStream.tellg();
    result.resize(streamLength);
    fileStream.seekg(0, std::ios::beg);
    fileStream.read((char*)result.data(), streamLength);
    if (fileStream.bad() || fileStream.fail()) {
      return tl::make_unexpected(path_error::ReadProblem);
    }
    return result;
  }
  return tl::make_unexpected(path_error::UnknownProblem);
}

template <typename T>
inline tl::expected<void, path_error> writeBinaryFile(
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
}  // namespace IO