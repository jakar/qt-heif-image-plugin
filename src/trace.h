#ifndef HEIF_IMAGE_PLUGIN_TRACE_H_
#define HEIF_IMAGE_PLUGIN_TRACE_H_

#include <QDebug>

#if defined(__GNUC__)
# define HEIF_IMAGE_PLUGIN_TRACE_CURRENT_FUNC __PRETTY_FUNCTION__
#else
# define HEIF_IMAGE_PLUGIN_TRACE_CURRENT_FUNC __func__
#endif

/**
 * \def HEIF_IMAGE_PLUGIN_TRACE(expr)
 *
 * Disabled by default, prints a debug message including source location.
 *
 * For this to evaluate expressions and print, HEIF_IMAGE_PLUGIN_TRACE_ENABLED
 * must be enabled at compile time, which is no the default case. If not
 * explicitly enabled, this is simply a no-op.
 */
#if defined(HEIF_IMAGE_PLUGIN_ENABLE_TRACE)
# define HEIF_IMAGE_PLUGIN_TRACE(...) \
    qDebug() \
      << "trace:" << HEIF_IMAGE_PLUGIN_TRACE_CURRENT_FUNC << ":" \
      << __FILE__ << ": line" << __LINE__ << ":" << __VA_ARGS__;
#else
# define HEIF_IMAGE_PLUGIN_TRACE(...)
#endif

#endif  // HEIF_IMAGE_PLUGIN_TRACE_H_
