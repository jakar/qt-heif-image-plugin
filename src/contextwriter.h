#ifndef HEIF_IMAGE_PLUGIN_CONTEXTWRITER_H_
#define HEIF_IMAGE_PLUGIN_CONTEXTWRITER_H_

#include <libheif/heif_cxx.h>

#include <QIODevice>

namespace heif_image_plugin {

class ContextWriter : public heif::Context::Writer
{
 public:
  explicit ContextWriter(QIODevice& device);

  virtual ~ContextWriter() = default;

  heif_error write(const void* data, size_t size) override;

 private:
  QIODevice& _device;
};

}  // namespace heif_image_plugin

#endif  // HEIF_IMAGE_PLUGIN_CONTEXTWRITER_H_
