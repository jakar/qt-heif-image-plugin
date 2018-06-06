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

void IOHandler::updateDevice()
{
  if (!device())
  {
    qWarning() << "device is null";
    Q_ASSERT(_context == nullptr);
  }

  if (device() != _device)
  {
    _device = device();
    _context.reset();
  }
}

void IOHandler::loadContext()
{
  updateDevice();

  if (_context || !device())
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

  _context = std::move(context);
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

    if (!_context)
    {
      qWarning() << "null context during read";
      return false;
    }

    auto handle = _context->get_primary_image_handle();
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

  if (origImage.isNull())
  {
    qWarning() << "origImage to write is null";
    return false;
  }

  updateDevice();

  if (_context)
  {
    qWarning() << "context not null before write";
    return false;
  }

  if (!device())
  {
    qWarning() << "device null before write";
    return false;
  }

  QImage qimage = origImage.convertToFormat(QImage::Format_RGBA8888);

  try
  {
    _context = std::make_unique<heif::Context>();

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

    // returns ImageHandle
    _context->encode_image(himage, encoder);

    ContextWriter writer(*device());
    _context->write(writer);

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
  Q_UNUSED(option_);
  HEIF_IMAGE_PLUGIN_TRACE("option:" << option_);
  return {};
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

      if (ok && q >=0 && q <= 100)
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

  return Quality;
}

}  // namespace heif_image_plugin
