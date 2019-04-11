#ifndef LOGGER_H_
#define LOGGER_H_

namespace Logger
{
  enum class Level
  {
    INFO,
    DEBUG,
    ERROR,
  };

  void log(const char* filename, int line, Level level, ...);
}

#define LOG_INFO(...)  do { Logger::log(__FILE__, __LINE__, Logger::Level::INFO,  __VA_ARGS__); } while(0)
#define LOG_DEBUG(...) do { Logger::log(__FILE__, __LINE__, Logger::Level::DEBUG, __VA_ARGS__); } while(0)
#define LOG_ERROR(...) do { Logger::log(__FILE__, __LINE__, Logger::Level::ERROR, __VA_ARGS__); } while(0)

#endif  // LOGGER_H_
