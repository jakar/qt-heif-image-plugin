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

#   ifndef HEIFIMAGEPLUGIN_LOGGING_LEVEL
#     if defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE)
#       define HEIFIMAGEPLUGIN_LOGGING_LEVEL spdlog::level::trace
#     elif defined(HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG)
#       define HEIFIMAGEPLUGIN_LOGGING_LEVEL spdlog::level::debug
#     else
#       define HEIFIMAGEPLUGIN_LOGGING_LEVEL spdlog::level::info
#     endif
#   endif

#   define HEIFIMAGEPLUGIN_LOG_DECLARE(var_name) \
      std::shared_ptr<spdlog::logger> var_name

#   define HEIFIMAGEPLUGIN_LOG_INIT(var_name, log_name) \
      var_name = spdlog::stdout_color_mt(log_name); \
      var_name->set_level(HEIFIMAGEPLUGIN_LOGGING_LEVEL); \
      var_name->set_pattern("%^[%n] [%l] %v%$")

#   define HEIFIMAGEPLUGIN_LOG_DO(expr) expr

# else

#   define HEIFIMAGEPLUGIN_LOG_DECLARE(var_name) \
      bool var_name = false

#   define HEIFIMAGEPLUGIN_LOG_INIT(var_name, log_name) \
      var_name = true

#   define HEIFIMAGEPLUGIN_LOG_DO(expr)

# endif

namespace heifimageplugin::log
{
# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING
  using LoggerPtr = std::shared_ptr<spdlog::logger>;
  using Level = spdlog::level::level_enum;
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

  template<class L, class... As>
  void log([[maybe_unused]] L&& logger,
           [[maybe_unused]] Level level,
           [[maybe_unused]] std::string_view const message,
           [[maybe_unused]] As&&... args)
  {
    HEIFIMAGEPLUGIN_LOG_DO(
      std::forward<L>(logger)->log(level, message.data(),
                                   convertArg(std::forward<As>(args))...);
    );
  }

  template<class L, class... As>
  void log([[maybe_unused]] L&& logger,
           [[maybe_unused]] Level level,
           [[maybe_unused]] char const* message,
           [[maybe_unused]] As&&... args)
  {
    HEIFIMAGEPLUGIN_LOG_DO(
      std::forward<L>(logger)->log(level, message,
                                   convertArg(std::forward<As>(args))...);
    );
  }

  template<class L, class M, class... As>
  void trace(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::trace,
        std::forward<M>(message), std::forward<As>(args)...);
  }

  template<class L, class M, class... As>
  void debug(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::debug,
        std::forward<M>(message), std::forward<As>(args)...);
  }

  template<class L, class M, class... As>
  void info(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::info,
        std::forward<M>(message), std::forward<As>(args)...);
  }

  template<class L, class M, class... As>
  void warn(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::warn,
        std::forward<M>(message), std::forward<As>(args)...);
  }

  template<class L, class M, class... As>
  void error(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::err,
        std::forward<M>(message), std::forward<As>(args)...);
  }

  template<class L, class M, class... As>
  void critical(L&& logger, M&& message, As&&... args)
  {
    log(std::forward<L>(logger), Level::critical,
        std::forward<M>(message), std::forward<As>(args)...);
  }
}

# ifdef HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE
#   define HEIFIMAGEPLUGIN_TRACE(logger, ...) \
      ::heifimageplugin::log::trace( \
        logger, \
        "[" + std::string(__PRETTY_FUNCTION__) + "] " \
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
