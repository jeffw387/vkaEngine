#pragma once
#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include <outcome.hpp>
#include <system_error>
#include <string>
#include "Logger.hpp"

enum class BinaryLoadError {
  Success = 0,
  PathProblem,
  ReadProblem,
  UnknownProblem
};

namespace std {
template <>
struct is_error_code_enum<BinaryLoadError> : std::true_type {};
}  // namespace std

namespace detail {
class BinaryLoadError_category : public std::error_category {
public:
  virtual const char* name() const noexcept override final {
    return "Binary file load error";
  }
  virtual std::string message(int c) const override final {
    switch (static_cast<BinaryLoadError>(c)) {
      case BinaryLoadError::Success:
        return "Success";
      case BinaryLoadError::PathProblem:
        return "Problem with the binary file path";
      case BinaryLoadError::ReadProblem:
        return "Problem reading from the binary file";
      case BinaryLoadError::UnknownProblem:
      default:
        return "Unknown error while loading binary file";
    }
  }

  virtual std::error_condition default_error_condition(int c) const
      noexcept override final {
    switch (static_cast<BinaryLoadError>(c)) {
      case BinaryLoadError::PathProblem:
        return make_error_condition(std::errc::no_such_file_or_directory);
      default:
        return std::error_condition(c, *this);
    }
  }
};
}  // namespace detail

extern inline const detail::BinaryLoadError_category&
BinaryLoadError_category() {
  static detail::BinaryLoadError_category c;
  return c;
}

inline std::error_code make_error_code(BinaryLoadError e) {
  return {static_cast<int>(e), BinaryLoadError_category()};
}

namespace vka {
namespace fs = std::experimental::filesystem;
namespace outcome = OUTCOME_V2_NAMESPACE;
inline outcome::result<std::vector<uint8_t>, BinaryLoadError> loadBinaryFile(
    fs::path binaryPath) {
  if (!fs::exists(binaryPath)) {
    return BinaryLoadError::PathProblem;
  }
  std::ifstream fileStream(
      binaryPath,
      std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
  size_t streamLength{};
  std::vector<uint8_t> result;
  if (fileStream) {
    streamLength = fileStream.tellg();
    result.resize(streamLength);
    fileStream.seekg(0, std::ios::beg);
    fileStream.read((char*)result.data(), streamLength);
    if (fileStream.bad() || fileStream.fail()) {
      return BinaryLoadError::ReadProblem;
    }
    return result;
  }
  return BinaryLoadError::UnknownProblem;
}
}  // namespace vka