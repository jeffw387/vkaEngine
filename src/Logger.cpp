#include "Logger.hpp"

std::shared_ptr<spdlog::sinks::sink> MultiLogger::fileSink = {};
std::shared_ptr<spdlog::sinks::sink> MultiLogger::stdoutSink = {};
std::shared_ptr<spdlog::sinks::sink> MultiLogger::stderrSink = {};
std::shared_ptr<spdlog::logger> MultiLogger::multilogger = {};

std::shared_ptr<spdlog::logger> MultiLogger::get() {
  if (!MultiLogger::multilogger) {
    MultiLogger::fileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFileName);
    MultiLogger::fileSink->set_level(spdlog::level::trace);
    MultiLogger::stdoutSink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    MultiLogger::stdoutSink->set_level(spdlog::level::info);
    MultiLogger::stderrSink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    MultiLogger::stderrSink->set_level(spdlog::level::info);
    std::vector<spdlog::sink_ptr> sinks = {MultiLogger::fileSink,
                                           MultiLogger::stderrSink};
    MultiLogger::multilogger = std::make_shared<spdlog::logger>(
        "MultiLogger", sinks.begin(), sinks.end());
    spdlog::register_logger(MultiLogger::multilogger);
    MultiLogger::multilogger->flush_on(spdlog::level::err);
  }
  return MultiLogger::multilogger;
}