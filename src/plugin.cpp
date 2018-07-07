#include "plugin.h"

#include "iohandler.h"

namespace qtheifimageplugin {

Plugin::Capabilities Plugin::capabilities(QIODevice* device,
                                          const QByteArray& format) const
{
    const bool formatOK = (format == "heic" || format == "heics"
                           || format == "heif" || format == "heifs");

    if (!formatOK && !format.isEmpty()) {
        return {};
    }

    if (device == nullptr) {
        if (formatOK) {
            return CanRead | CanWrite;
        } else {
            return {};
        }
    }

    using F = IOHandler::Format;
    Capabilities caps{};

    if (device->isReadable() && IOHandler::canReadFrom(*device) != F::None) {
        caps |= CanRead;
    }

    if (device->isWritable()) {
        caps |= CanWrite;
    }

    return caps;
}

QImageIOHandler* Plugin::create(QIODevice* device,
                                const QByteArray& format) const
{
    IOHandler* ioHandler = new IOHandler();
    ioHandler->setDevice(device);
    ioHandler->setFormat(format);
    return ioHandler;
}

}  // namespace qtheifimageplugin
