#pragma once
#include <spdlog/spdlog.h>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>

static auto LogFileName = "vkaEngineLog.txt";

class MultiLogger {
  static std::shared_ptr<spdlog::sinks::sink> fileSink;
  static std::shared_ptr<spdlog::sinks::sink> stdoutSink;
  static std::shared_ptr<spdlog::sinks::sink> stderrSink;
  static std::shared_ptr<spdlog::logger> multilogger;

  MultiLogger() {}

public:
  static std::shared_ptr<spdlog::logger> get();
};
