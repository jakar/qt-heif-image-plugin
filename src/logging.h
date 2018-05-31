#ifndef HEIFIMAGEPLUGIN_LOGGING_H_
#define HEIFIMAGEPLUGIN_LOGGING_H_

#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

# if defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE) \
      && !defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG)
#   define HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG
# endif

# if defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG) \
      && !defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING)
#   define HEIFIMAGEPLUGIN_ENABLE_LOGGING
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG
#   define SPDLOG_DEBUG_ON
# endif
# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE
#   define SPDLOG_TRACE_ON
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING

#   include <spdlog/spdlog.h>
#   include <spdlog/fmt/ostr.h>
#   include <boost/current_function.hpp>

#   define HEIFIMAGEPLUGIN_LOG_DECLARE(logger) \
      std::shared_ptr<spdlog::logger> logger

#   define HEIFIMAGEPLUGIN_LOG_INIT(logger) \
      logger = spdlog::stdout_color_mt(\
        ::heifimageplugin::log::loggerName.data()); \
      logger->set_level(::heifimageplugin::log::visibleLevel); \
      logger->set_pattern(::heifimageplugin::log::pattern.data())

#   define HEIFIMAGEPLUGIN_LOG_DO(expr) expr

# else

#   define HEIFIMAGEPLUGIN_LOG_DECLARE(logger) \
      bool logger = false

#   define HEIFIMAGEPLUGIN_LOG_INIT(logger) \
      logger = true

#   define HEIFIMAGEPLUGIN_LOG_DO(expr)

# endif

namespace heifimageplugin::log
{
# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
  using namespace std::literals;

  using LoggerPtr = std::shared_ptr<spdlog::logger>;
  using Level = spdlog::level::level_enum;

  constexpr std::string_view loggerName = "heifimageplugin"sv;
  constexpr std::string_view pattern = "%^[%n] [%l] %v%$"sv;
# else
  using LoggerPtr = bool;

  enum class Level
  {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
  };
# endif

# if defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE)
  constexpr Level visibleLevel = Level::trace;
# elif defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG)
  constexpr Level visibleLevel = Level::debug;
# else
  constexpr Level visibleLevel = Level::info;
# endif

  template<class A>
  decltype(auto) convertArg(A&& arg)
  {
    if constexpr (std::is_pointer_v<std::decay_t<A>>)
    {
      return static_cast<void*>(arg);
    }
    else
    {
      return std::forward<A>(arg);
    }
  }

  template<class... As>
  void log([[maybe_unused]] Level level,
           [[maybe_unused]] LoggerPtr const& logger,
           [[maybe_unused]] std::string_view const& message,
           [[maybe_unused]] As&&... args)
  {
    HEIFIMAGEPLUGIN_LOG_DO(
      logger->log(level, message.data(),
                  convertArg(std::forward<As>(args))...);
    );
  }

  template<class... As>
  void log([[maybe_unused]] Level level,
           [[maybe_unused]] std::string_view const& message,
           [[maybe_unused]] As&&... args)
  {
    HEIFIMAGEPLUGIN_LOG_DO(
      auto logger = spdlog::get(loggerName.data());
      if (logger)
        log(level, logger, message, std::forward<As>(args)...);
    );
  }

  template<class... As>
  void trace(As&&... args)
  {
    log(Level::trace, std::forward<As>(args)...);
  }

  template<class... As>
  void debug(As&&... args)
  {
    log(Level::debug, std::forward<As>(args)...);
  }

  template<class... As>
  void info(As&&... args)
  {
    log(Level::info, std::forward<As>(args)...);
  }

  template<class... As>
  void warn(As&&... args)
  {
    log(Level::warn, std::forward<As>(args)...);
  }

  template<class... As>
  void error(As&&... args)
  {
    log(Level::err, std::forward<As>(args)...);
  }

  template<class... As>
  void critical(As&&... args)
  {
    log(Level::critical, std::forward<As>(args)...);
  }
}

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE
#   define HEIFIMAGEPLUGIN_TRACE(logger, ...) \
      ::heifimageplugin::log::trace( \
        logger, \
        "[" + std::string(BOOST_CURRENT_FUNCTION) + "] " \
        "[" __FILE__ ":" SPDLOG_STR_HELPER(__LINE__) "] " \
        __VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_TRACE(...)
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG
#   define HEIFIMAGEPLUGIN_DEBUG(...) \
      ::heifimageplugin::log::debug(__VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_DEBUG(...)
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
#   define HEIFIMAGEPLUGIN_INFO(...) \
      ::heifimageplugin::log::info(__VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_INFO(...)
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
#   define HEIFIMAGEPLUGIN_WARN(...) \
      ::heifimageplugin::log::warn(__VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_WARN(...)
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
#   define HEIFIMAGEPLUGIN_ERROR(...) \
      ::heifimageplugin::log::error(__VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_ERROR(...)
# endif

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
#   define HEIFIMAGEPLUGIN_CRITICAL(...) \
      ::heifimageplugin::log::critical(__VA_ARGS__)
# else
#   define HEIFIMAGEPLUGIN_CRITICAL(...)
# endif

#endif // HEIFIMAGEPLUGIN_LOGGING_H_
