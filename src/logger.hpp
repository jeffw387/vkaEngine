#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

static auto logFileName = "vkaEngineLog.txt";

class multi_logger {
  static std::shared_ptr<spdlog::sinks::sink> fileSink;
  static std::shared_ptr<spdlog::sinks::sink> stdoutSink;
  static std::shared_ptr<spdlog::sinks::sink> stderrSink;
  static std::shared_ptr<spdlog::logger> multilogger;

  multi_logger() = default;

public:
  static std::shared_ptr<spdlog::logger> get();
};
