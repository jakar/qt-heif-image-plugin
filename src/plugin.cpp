#include "plugin.h"

#include "iohandler.h"

namespace heif_image_plugin {

Plugin::Plugin(QObject* parent_)
  : QImageIOPlugin(parent_)
{
}

Plugin::~Plugin()
{
}

Plugin::Capabilities Plugin::capabilities(QIODevice* device,
                                          const QByteArray& format) const
{
  const bool formatOK = (format == "heic" || format == "heif");

  if (!formatOK && !format.isEmpty())
  {
    return {};
  }

  if (device == nullptr)
  {
    if (formatOK)
    {
      return CanRead;
    }
    else
    {
      return {};
    }
  }

  if (device->isReadable() && IOHandler::canReadFrom(*device))
  {
    return CanRead;
  }

  return {};
}

QImageIOHandler* Plugin::create(QIODevice* device,
                                const QByteArray& format) const
{
  IOHandler* ioHandler = new IOHandler();
  ioHandler->setDevice(device);
  ioHandler->setFormat(format);
  return ioHandler;
}

}  // namespace heif_image_plugin
