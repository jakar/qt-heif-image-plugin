#ifndef QTHEIFIMAGEPLUGIN_UTIL_H_
#define QTHEIFIMAGEPLUGIN_UTIL_H_

#include <memory>
#include <utility>

namespace qtheifimageplugin {
namespace util {

#if defined(__cpp_lib_make_unique) && __cpp_lib_make_unique >= 201304
using std::make_unique;
#else
template<class T, class... As>
std::unique_ptr<T> make_unique(As&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<As>(args)...));
}
#endif

template<class T, class... As>
std::unique_ptr<T> make_unique_aggregate(As&&... args)
{
  return std::unique_ptr<T>(new T{std::forward<As>(args)...});
}

}  // namespace util
}  // namespace qtheifimageplugin

#endif  // QTHEIFIMAGEPLUGIN_UTIL_H_
