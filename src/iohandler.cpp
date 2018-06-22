#include "iohandler.h"

#include "contextwriter.h"

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
#include "trace.h"

namespace {

constexpr int kDefaultQuality = 50;  // TODO: maybe adjust this

}  // namespace

namespace heif_image_plugin {

IOHandler::IOHandler()
  : QImageIOHandler(),
    _quality{kDefaultQuality}
{
  HEIF_IMAGE_PLUGIN_TRACE("");
}

IOHandler::~IOHandler()
{
  HEIF_IMAGE_PLUGIN_TRACE("");
}

void IOHandler::updateDevice()
{
  if (!device())
  {
    qWarning() << "device is null";
    Q_ASSERT(_readState == nullptr);
  }

  if (device() != _device)
  {
    _device = device();
    _readState.reset();
  }
}

//
// Peeking
//

bool IOHandler::canReadFrom(QIODevice& device)
{
  HEIF_IMAGE_PLUGIN_TRACE("");

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

void IOHandler::loadContext()
{
  updateDevice();

  if (!device())
  {
    return;
  }

  if (_readState)
  {
    // context already loded
    return;
  }

  auto rs = util::make_unique_aggregate<ReadState>(device()->readAll());
  const auto& fileData = rs->fileData;

  if (fileData.isEmpty())
  {
    qWarning() << "failed to read file data";
    return;
  }

  // set up new context
  rs->context.read_from_memory_without_copy(fileData.data(), fileData.size());
  rs->handle = rs->context.get_primary_image_handle();

  rs->image = rs->handle.decode_image(heif_colorspace_RGB,
                                      heif_chroma_interleaved_RGBA);

  auto chan = heif_channel_interleaved;
  rs->size = QSize(rs->image.get_width(chan), rs->image.get_height(chan));

  if (!rs->size.isValid())
  {
    qDebug() << "invalid image size:"
      << rs->size.width() << "x" << rs->size.height();
    return;
  }

  _readState = std::move(rs);
}

bool IOHandler::read(QImage* qimage)
{
  HEIF_IMAGE_PLUGIN_TRACE("qimage:" << qimage);

  if (!qimage)
  {
    qWarning() << "image is null";
    return false;
  }

  try
  {
    loadContext();

    if (!_readState)
    {
      qWarning() << "failed to decode image";
      return false;
    }

    auto& himage = _readState->image;
    const auto& imgSize = _readState->size;
    auto channel = heif_channel_interleaved;

    int stride = 0;
    const uint8_t* data = himage.get_plane(channel, &stride);

    if (!data)
    {
      qWarning() << "pixel data not found";
      return false;
    }

    if (stride <= 0)
    {
      qWarning() << "invalid stride:" << stride;
      return false;
    }

    // copy image data
    int dataSize = imgSize.height() * stride;
    uint8_t* dataCopy = new uint8_t[dataSize];

    std::copy(data, data + dataSize, dataCopy);

    *qimage = QImage(
      dataCopy, imgSize.width(), imgSize.height(),
      stride, QImage::Format_RGBA8888,
      [](void* d) { delete[] static_cast<uint8_t*>(d); },
      dataCopy
    );

    return true;
  }
  catch (const heif::Error& error)
  {
    qWarning() << "libheif read error:" << error.get_message().c_str();
  }

  return false;
}

//
// Writing
//

bool IOHandler::write(const QImage& origImage)
{
  HEIF_IMAGE_PLUGIN_TRACE("");

  updateDevice();

  if (!device())
  {
    qWarning() << "device null before write";
    return false;
  }

  if (origImage.isNull())
  {
    qWarning() << "image to write is null";
    return false;
  }

  QImage qimage = origImage.convertToFormat(QImage::Format_RGBA8888);

  try
  {
    heif::Context context{};

    heif::Encoder encoder(heif_compression_HEVC);
    encoder.set_lossy_quality(_quality);

    int width = qimage.width();
    int height = qimage.height();

    heif::Image himage{};
    himage.create(width, height,
                  heif_colorspace_RGB,
                  heif_chroma_interleaved_RGBA);

    auto channel = heif_channel_interleaved;
    himage.add_plane(channel, width, height, 32);

    int himgStride;
    uint8_t* himgData = himage.get_plane(channel, &himgStride);

    const uint8_t* qimgData = qimage.bits();
    const int qimgStride = qimage.bytesPerLine();

    if (qimage.bytesPerLine() > himgStride)
    {
      qWarning() << "source line larger than destination";
      return false;
    }

    for (int y = 0; y < height; ++y)
    {
      auto* qimgBegin = qimgData + y * qimgStride;
      auto* qimgEnd = qimgBegin + qimgStride;
      std::copy(qimgBegin, qimgEnd, himgData + y * himgStride);
    }

    context.encode_image(himage, encoder);

    ContextWriter writer(*device());
    context.write(writer);

    return true;
  }
  catch (const heif::Error& error)
  {
    qWarning() << "libheif write error:" << error.get_message().c_str();
  }

  return false;
}

//
// Options
//

QVariant IOHandler::option(ImageOption option_) const
{
  HEIF_IMAGE_PLUGIN_TRACE("option:" << option_);

  switch (option_)
  {
    case Size:
      return _readState ? _readState->size : QVariant{};

    default:
      return {};
  }
}

void IOHandler::setOption(ImageOption option_, const QVariant& value)
{
  HEIF_IMAGE_PLUGIN_TRACE("option:" << option_ << ", value:" << value);

  switch (option_)
  {
    case Quality:
    {
      bool ok = false;
      int q = value.toInt(&ok);

      if (ok && q >= 0 && q <= 100)
      {
        _quality = q;
      }
    }

    default:
      return;
  }
}

bool IOHandler::supportsOption(ImageOption option_) const
{
  HEIF_IMAGE_PLUGIN_TRACE("option:" << option_);

  return option_ == Quality || option_ == Size;
}

}  // namespace heif_image_plugin
