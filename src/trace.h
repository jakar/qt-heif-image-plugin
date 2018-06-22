#ifndef QTHEIFIMAGEPLUGIN_TRACE_H_
#define QTHEIFIMAGEPLUGIN_TRACE_H_

#include <QDebug>

#if defined(__GNUC__)
# define QTHEIFIMAGEPLUGIN_TRACE_CURRENT_FUNC __PRETTY_FUNCTION__
#else
# define QTHEIFIMAGEPLUGIN_TRACE_CURRENT_FUNC __func__
#endif

/**
 * \def QTHEIFIMAGEPLUGIN_TRACE(expr)
 *
 * Disabled by default, prints a debug message including source location.
 *
 * For this to evaluate expressions and print, QTHEIFIMAGEPLUGIN_TRACE_ENABLED
 * must be enabled at compile time, which is no the default case. If not
 * explicitly enabled, this is simply a no-op.
 */
#if defined(QTHEIFIMAGEPLUGIN_ENABLE_TRACE)
# define QTHEIFIMAGEPLUGIN_TRACE(...) \
    qDebug() \
      << "trace:" << QTHEIFIMAGEPLUGIN_TRACE_CURRENT_FUNC << ":" \
      << __FILE__ << ": line" << __LINE__ << ":" << __VA_ARGS__;
#else
# define QTHEIFIMAGEPLUGIN_TRACE(...)
#endif

#endif  // QTHEIFIMAGEPLUGIN_TRACE_H_
