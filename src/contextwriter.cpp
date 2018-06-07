#include "contextwriter.h"

#include <QDebug>

#include <cstdint>

namespace {

constexpr heif_error kOkError{
  heif_error_Ok,
  heif_suberror_Unspecified,
  "ok"
};

constexpr heif_error kWriteError{
  heif_error_Encoding_error,
  heif_suberror_Cannot_write_output_data,
  "write failed"
};

}  // namespace

namespace heif_image_plugin {

ContextWriter::ContextWriter(QIODevice& device)
  : _device(device)
{
}

heif_error ContextWriter::write(const void* data, size_t size)
{
  auto bytesWritten = _device.write(static_cast<const char*>(data), size);

  if (bytesWritten != static_cast<int64_t>(size))
  {
    qWarning()
      << "write failed:"
      << bytesWritten << "/" << size << "bytes written";

    return kWriteError;
  }

  return kOkError;
}

}  // namespace heif_image_plugin
