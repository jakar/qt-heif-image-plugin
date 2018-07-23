#ifndef QTHEIFIMAGEPLUGIN_LOG_H_
#define QTHEIFIMAGEPLUGIN_LOG_H_

#include <QDebug>

#if defined(__GNUC__)
#   define QTHEIFIMAGEPLUGIN_LOG_CURRENT_FUNC __PRETTY_FUNCTION__
#else
#   define QTHEIFIMAGEPLUGIN_LOG_CURRENT_FUNC __func__
#endif

namespace qtheifimageplugin {
namespace log {

namespace detail {

inline constexpr const char* prefix()
{
    return "qt-heif-image-plugin: ";
}

inline QDebug print(QDebug debug)
{
    debug.nospace() << prefix();
    return debug;
}

}  // namespace detail

inline QDebug debug()
{
    return detail::print(qDebug());
}

inline QDebug warning()
{
    return detail::print(qWarning());
}

inline QDebug critical()
{
    return detail::print(qCritical());
}

namespace detail {

inline QDebug trace(const char* func, const char* file, long line)
{
    return debug() << "trace: [" << func << "] [" << file << ":" << line << "] ";
}

}  // namespace detail

}  // namespace log
}  // namespace qtheifimageplugin

/**
 * \def QTHEIFIMAGEPLUGIN_LOG_TRACE(expr)
 *
 * Disabled by default, prints a debug message including source location.
 *
 * For this to evaluate expressions and print, QTHEIFIMAGEPLUGIN_ENABLE_TRACE
 * must be enabled at compile time, which is no the default case. Otherwise,
 * this is simply a no-op.
 */
#if defined(QTHEIFIMAGEPLUGIN_ENABLE_TRACE)
#   define QTHEIFIMAGEPLUGIN_LOG_TRACE(...) \
        ::qtheifimageplugin::log::detail::trace( \
            QTHEIFIMAGEPLUGIN_LOG_CURRENT_FUNC, __FILE__, __LINE__) << __VA_ARGS__;
#else
#   define QTHEIFIMAGEPLUGIN_LOG_TRACE(...)
#endif

#endif  // QTHEIFIMAGEPLUGIN_LOG_H_
