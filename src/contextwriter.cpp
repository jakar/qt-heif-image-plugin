#include "contextwriter.h"

#include "log.h"

#include <type_traits>

namespace qtheifimageplugin {

ContextWriter::ContextWriter(QIODevice& device) :
    _device(device)
{
}

heif_error ContextWriter::write(const void* data, size_t size)
{
    qint64 bytesWritten = _device.write(static_cast<const char*>(data), size);

    using I = typename std::conditional<sizeof(size_t) >= sizeof(qint64),
                                        size_t,
                                        qint64>::type;

    if (bytesWritten < 0 || static_cast<I>(bytesWritten) != static_cast<I>(size)) {
        log::warning() << "write failed: "
            << bytesWritten << " / " << size << " bytes written";

        return {
            heif_error_Encoding_error,
            heif_suberror_Cannot_write_output_data,
            "write failed"
        };
    }

    return {
        heif_error_Ok,
        heif_suberror_Unspecified,
        "ok"
    };
}

}  // namespace qtheifimageplugin
