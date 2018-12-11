#pragma once
#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include <outcome.hpp>
#include <system_error>
#include <string>
#include "gsl-lite.hpp"
#include "Logger.hpp"

enum class IOError {
  Success = 0,
  PathProblem,
  ReadProblem,
  WriteProblem,
  UnknownProblem
};

namespace std {
template <>
struct is_error_code_enum<IOError> : std::true_type {};
}  // namespace std

namespace detail {
class IOError_category : public std::error_category {
public:
  virtual const char* name() const noexcept override final {
    return "IO error";
  }
  virtual std::string message(int c) const override final {
    switch (static_cast<IOError>(c)) {
      case IOError::Success:
        return "Success";
      case IOError::PathProblem:
        return "Problem with the file path";
      case IOError::ReadProblem:
        return "Problem reading from the file";
      case IOError::WriteProblem:
        return "Problem writing to the file";
      case IOError::UnknownProblem:
      default:
        return "Unknown IO error";
    }
  }

  virtual std::error_condition default_error_condition(int c) const
      noexcept override final {
    switch (static_cast<IOError>(c)) {
      case IOError::PathProblem:
        return make_error_condition(std::errc::no_such_file_or_directory);
      default:
        return std::error_condition(c, *this);
    }
  }
};
}  // namespace detail

extern inline const detail::IOError_category& IOError_category() {
  static detail::IOError_category c;
  return c;
}

inline std::error_code make_error_code(IOError e) {
  return {static_cast<int>(e), IOError_category()};
}

namespace vka {
namespace fs = std::experimental::filesystem;
namespace outcome = OUTCOME_V2_NAMESPACE;
inline outcome::result<std::vector<uint8_t>, IOError> loadBinaryFile(
    fs::path filePath) {
  if (!fs::exists(filePath)) {
    return IOError::PathProblem;
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
      return IOError::ReadProblem;
    }
    return result;
  }
  return IOError::UnknownProblem;
}

template <typename T>
inline IOError writeBinaryFile(fs::path filePath, gsl::span<T> data) {
  std::ofstream fileStream(
      filePath, std::ios_base::out | std::ios_base::binary);
  if (fileStream) {
    fileStream.seekp(0, std::ios::beg);
    auto writeSize = data.length_bytes();
    fileStream.write((char*)data.data(), writeSize);
    if (fileStream.bad() || fileStream.fail()) {
      return IOError::WriteProblem;
    }
    return IOError::Success;
  }
  return IOError::PathProblem;
}
}  // namespace vka