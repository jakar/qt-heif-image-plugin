#ifndef QTHEIFIMAGEPLUGIN_CONTEXTWRITER_H_
#define QTHEIFIMAGEPLUGIN_CONTEXTWRITER_H_

#include <libheif/heif_cxx.h>

#include <QIODevice>

namespace qtheifimageplugin {

class ContextWriter : public heif::Context::Writer
{
 public:
  explicit ContextWriter(QIODevice& device);

  virtual ~ContextWriter() = default;

  heif_error write(const void* data, size_t size) override;

 private:
  QIODevice& _device;
};

}  // namespace qtheifimageplugin

#endif  // QTHEIFIMAGEPLUGIN_CONTEXTWRITER_H_
