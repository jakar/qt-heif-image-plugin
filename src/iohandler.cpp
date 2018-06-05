#include "iohandler.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>

#include <libheif/heif_cxx.h>

#include <QDebug>
#include <QImage>
#include <QSize>
#include <QVariant>

#include "util.h"

namespace heif_image_plugin {

IOHandler::IOHandler()
  : QImageIOHandler()
{
}

IOHandler::~IOHandler() {}

//
// Peeking
//

bool IOHandler::canReadFrom(QIODevice& device)
{
  // logic taken from qt macheif plugin
  constexpr int kHeaderSize = 12;
  QByteArray header = device.peek(kHeaderSize);

  if (header.size() != kHeaderSize)
  {
    qWarning() << "could not read header";
    return false;
  }

  const QByteArray w1 = header.mid(4, 4);
  const QByteArray w2 = header.mid(8, 4);

  return w1 == "ftyp" && (w2 == "heic" || w2 == "heix" || w2 == "mifi");
}

bool IOHandler::canRead() const
{
  if (device() && canReadFrom(*device()))
  {
    setFormat("heic");  // bastardized const
    return true;
  }

  return false;
}

//
// Reading
//

void IOHandler::updateDevice()
{
  if (!device())
  {
    qWarning() << "device is null";
    Q_ASSERT(context_ == nullptr);
  }

  if (device() != device_)
  {
    device_ = device();
    context_.reset();
  }
}

void IOHandler::loadContext()
{
  updateDevice();

  if (context_ || !device())
  {
    return;
  }

  auto fileData = device()->readAll();

  if (fileData.isEmpty())
  {
    qWarning() << "failed to read file data";
    return;
  }

  auto context = util::make_unique<heif::Context>();
  context->read_from_memory(fileData.data(), fileData.size());

  context_ = std::move(context);
}

bool IOHandler::read(QImage* qimage)
{
  if (!qimage)
  {
    qWarning() << "image is null";
    return false;
  }

  try
  {
    loadContext();

    if (!context_)
    {
      qWarning() << "null context during read";
      return false;
    }

    auto handle = context_->get_primary_image_handle();
    auto himage = handle.decode_image(heif_colorspace_RGB,
                                      heif_chroma_interleaved_RGBA);

    auto channel = heif_channel_interleaved;
    int width = himage.get_width(channel);
    int height = himage.get_height(channel);

    int stride = 0;
    const uint8_t* data = himage.get_plane(channel, &stride);

    // copy image data
    int dataSize = height * stride;
    uint8_t* dataCopy = new uint8_t[dataSize];

    std::copy(data, data + dataSize, dataCopy);

    *qimage = QImage(
      dataCopy, width, height, stride, QImage::Format_RGBA8888,
      [](void* d) { delete[] static_cast<uint8_t*>(d); }
    );

    return true;
  }
  catch (const heif::Error& error)
  {
    qWarning() << "libheif read error: {}" << error.get_message().c_str();
  }

  return false;
}

//
// Options
//

QVariant IOHandler::option(ImageOption option) const
{
  Q_UNUSED(option);
  return {};
}

void IOHandler::setOption(ImageOption option, const QVariant& value)
{
  Q_UNUSED(option);
  Q_UNUSED(value);
}

bool IOHandler::supportsOption(ImageOption option) const
{
  Q_UNUSED(option);
  return false;
}

}  // namespace heif_image_plugin
