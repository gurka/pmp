#include "logger.h"

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <cstring>

void Logger::log(const char* filename, int line, Level level, ...)
{
#ifndef ENABLE_LOG_DEBUG
  // Only print LOG_DEBUGs if ENABLE_LOG_DEBUG is defined during build
  if (level == Level::DEBUG)
  {
    return;
  }
#endif

  // Remove directories in filename ("src/logger.cc" => "logger.cc")
  if (strrchr(filename, '/'))
  {
    filename = strrchr(filename, '/') + 1;
  }

  // Get current date and time
  time_t now = time(0);
  struct tm tstruct{};
  char time_str[32];
  localtime_r(&now, &tstruct);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %X", &tstruct);

  // Extract variadic function arguments
  va_list args;
  va_start(args, level);  // Start to extract after the "level"-argument
                          // which also must be a non-reference otherwise
                          // va_start invokes undefined behaviour
  const char* format = va_arg(args, const char*);
  char message[256];

  // Use the rest of the arguments together with the
  // format string to construct the actual log message
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  const auto level_to_str = [](Level level) -> const char*
  {
    switch (level)
    {
      case Level::INFO:  return "INFO";
      case Level::DEBUG: return "DEBUG";
      case Level::ERROR: return "ERROR";
      default:           return "INVALID_LEVEL";
    }
  };

  // Print and flush
  printf("[%s][%s:%d] %s: %s\n", time_str, filename, line, level_to_str(level), message);
  std::fflush(stdout);
}
