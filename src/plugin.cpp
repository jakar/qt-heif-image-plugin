#include "plugin.h"

#include "iohandler.h"

#include <iostream>

namespace heifimageplugin
{
  Plugin::Plugin(QObject* parent_)
  :
    QImageIOPlugin(parent_)
  {
    HEIFIMAGEPLUGIN_LOG_INIT(_log, "heifimageplugin");
    HEIFIMAGEPLUGIN_TRACE(_log);
  }

  Plugin::~Plugin()
  {
    HEIFIMAGEPLUGIN_TRACE(_log);
  }

  Plugin::Capabilities Plugin::capabilities(QIODevice* device,
                                            QByteArray const& format) const
  {
    HEIFIMAGEPLUGIN_TRACE(_log, "device: {}, format: {}",
                          device, format.toStdString());

    bool formatOK = (format == "heic" || format == "heif");

    if (!formatOK && !format.isEmpty())
      return {};

    if (!device)
    {
      if (formatOK)
        return CanRead;
      else
        return {};
    }

    if (device->isReadable() && IOHandler::canReadFrom(*device, _log))
      return CanRead;

    return {};
  }

  QImageIOHandler* Plugin::create(QIODevice* device,
                                  QByteArray const& format) const
  {
    HEIFIMAGEPLUGIN_TRACE(_log);

    IOHandler* ioHandler = new IOHandler(_log);
    ioHandler->setDevice(device);
    ioHandler->setFormat(format);
    return ioHandler;
  }
}
