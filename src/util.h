#ifndef HEIF_IMAGE_PLUGIN_UTIL_H_
#define HEIF_IMAGE_PLUGIN_UTIL_H_

#include <memory>
#include <utility>

namespace heif_image_plugin {
namespace util {

#if __cpp_lib_make_unique >= 201304
using std::make_unique;
#else
template<class T, class... As>
std::unique_ptr<T> make_unique(As&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<As>(args)...));
}
#endif

}  // namespace util
}  // namespace heif_image_plugin

#endif  // HEIF_IMAGE_PLUGIN_UTIL_H_
