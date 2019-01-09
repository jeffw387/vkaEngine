#include "Logger.hpp"

std::shared_ptr<spdlog::sinks::sink> multi_logger::fileSink = {};
std::shared_ptr<spdlog::sinks::sink> multi_logger::stdoutSink = {};
std::shared_ptr<spdlog::sinks::sink> multi_logger::stderrSink = {};
std::shared_ptr<spdlog::logger> multi_logger::multilogger = {};

std::shared_ptr<spdlog::logger> multi_logger::get() {
  if (!multi_logger::multilogger) {
    multi_logger::fileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName);
    multi_logger::fileSink->set_level(spdlog::level::trace);
    multi_logger::stdoutSink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    multi_logger::stdoutSink->set_level(spdlog::level::info);
    multi_logger::stderrSink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    multi_logger::stderrSink->set_level(spdlog::level::info);
    std::vector<spdlog::sink_ptr> sinks = {multi_logger::fileSink,
                                           multi_logger::stderrSink};
    multi_logger::multilogger = std::make_shared<spdlog::logger>(
        "vkaEngine", sinks.begin(), sinks.end());
    spdlog::register_logger(multi_logger::multilogger);
    multi_logger::multilogger->flush_on(spdlog::level::err);
  }
  return multi_logger::multilogger;
}