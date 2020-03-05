#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger {
  using string_view_t = fmt::basic_string_view<char>;
protected:
  std::shared_ptr<spdlog::logger> logger;
  std::string name_;
public:
  explicit Logger(std::string name) : name_ {name}, logger {spdlog::stderr_color_st(name)} {}
  template<typename... Args>
  void debug(string_view_t fmt, const Args &... args) {
    logger->debug(fmt, args...);
  }
  template<typename... Args>
  void info(string_view_t fmt, const Args &... args) {
    logger->info(fmt, args...);
  }
  template<typename... Args>
  void warn(string_view_t fmt, const Args &... args) {
    logger->warn(fmt, args...);
  }
  template<typename... Args>
  void error(string_view_t fmt, const Args &... args) {
    logger->error(fmt, args...);
  }
};

#endif
