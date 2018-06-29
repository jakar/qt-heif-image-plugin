#include "iohandler.h"

#include "contextwriter.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>

#include <libheif/heif_cxx.h>

#include <QImage>
#include <QSize>
#include <QVariant>

#include "log.h"
#include "util.h"

namespace {

constexpr int kDefaultQuality = 50;  // TODO: maybe adjust this

}  // namespace

namespace qtheifimageplugin {

IOHandler::IOHandler()
  : QImageIOHandler(),
    _quality{kDefaultQuality}
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("");
}

IOHandler::~IOHandler()
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("");
}

void IOHandler::updateDevice()
{
  if (!device())
  {
    log::warning() << "device is null";
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

IOHandler::Format IOHandler::canReadFrom(QIODevice& device)
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("");

  // read beginning of ftyp box at beginning of file
  constexpr int kHeaderSize = 12;
  QByteArray header = device.peek(kHeaderSize);

  if (header.size() != kHeaderSize)
  {
    return Format::none;
  }

  // skip first four bytes, which contain box size
  const QByteArray w1 = header.mid(4, 4);
  const QByteArray w2 = header.mid(8, 4);

  if (w1 != "ftyp")
  {
    // not an ftyp box
    return Format::none;
  }

  // brand follows box name, determines format
  if (w2 == "mif1")
  {
    return Format::heif;
  }
  else if (w2 == "msf1")
  {
    return Format::heifSequence;
  }
  else if (w2 == "heic" || w2 == "heix")
  {
    return Format::heic;
  }
  else if (w2 == "hevc" || w2 == "hevx")
  {
    return Format::heicSequence;
  }
  else
  {
    return Format::none;
  }
}

bool IOHandler::canRead() const
{
  if (!device())
  {
    return false;
  }

  auto format = canReadFrom(*device());

  // Other image plugins set the format here. Not sure if it is really
  // necessary or what it accomplishes.
  switch (format)
  {
    case Format::heif:
      setFormat("heif");
      return true;

    case Format::heifSequence:
      setFormat("heifs");
      return true;

    case Format::heic:
      setFormat("heic");
      return true;

    case Format::heicSequence:
      setFormat("heics");
      return true;

    default:
      return false;
  }
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
    log::debug() << "failed to read file data";
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
    log::debug() << "invalid image size: "
      << rs->size.width() << "x" << rs->size.height();
    return;
  }

  _readState = std::move(rs);
}

bool IOHandler::read(QImage* qimage)
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("");

  if (!qimage)
  {
    log::warning() << "QImage to read into is null";
    return false;
  }

  try
  {
    loadContext();

    if (!_readState)
    {
      log::debug() << "failed to decode image";
      return false;
    }

    auto& himage = _readState->image;
    const auto& imgSize = _readState->size;
    auto channel = heif_channel_interleaved;

    int stride = 0;
    const uint8_t* data = himage.get_plane(channel, &stride);

    if (!data)
    {
      log::warning() << "pixel data not found";
      return false;
    }

    if (stride <= 0)
    {
      log::warning() << "invalid stride: " << stride;
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
    log::warning() << "libheif read error: " << error.get_message().c_str();
  }

  return false;
}

//
// Writing
//

bool IOHandler::write(const QImage& origImage)
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("");

  updateDevice();

  if (!device())
  {
    log::warning() << "device null before write";
    return false;
  }

  if (origImage.isNull())
  {
    log::warning() << "source image is null";
    return false;
  }

  QImage qimage = origImage.convertToFormat(QImage::Format_RGBA8888);

  if (qimage.isNull())
  {
    log::warning() << "source image format conversion failed";
    return false;
  }

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

    int himgStride = 0;
    uint8_t* himgData = himage.get_plane(channel, &himgStride);

    if (!himgData)
    {
      log::warning() << "could not get libheif image plane";
      return false;
    }

    if (himgStride <= 0)
    {
      log::warning() << "invalid destination stride: " << himgStride;
      return false;
    }

    const uint8_t* qimgData = qimage.constBits();
    const int qimgStride = qimage.bytesPerLine();

    if (!qimgData)
    {
      log::warning() << "source image data is null";
      return false;
    }

    if (qimgStride <= 0)
    {
      log::warning() << "invalid source image stride: " << qimgStride;
      return false;
    }
    else if (qimgStride > himgStride)
    {
      log::warning() << "source line larger than destination";
      return false;
    }

    // copy rgba data
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
    log::warning() << "libheif write error: " << error.get_message().c_str();
  }

  return false;
}

//
// Options
//

QVariant IOHandler::option(ImageOption opt) const
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("opt: " << opt);

  switch (opt)
  {
    case Size:
      return _readState ? _readState->size : QVariant{};

    default:
      return {};
  }
}

void IOHandler::setOption(ImageOption opt, const QVariant& value)
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("opt: " << opt << ", value: " << value);

  switch (opt)
  {
    case Quality:
    {
      bool ok = false;
      int q = value.toInt(&ok);

      if (ok && q >= 0 && q <= 100)
      {
        _quality = q;
      }

      return;
    }

    default:
      return;
  }
}

bool IOHandler::supportsOption(ImageOption opt) const
{
  QTHEIFIMAGEPLUGIN_LOG_TRACE("opt: " << opt);

  return opt == Quality || opt == Size;
}

}  // namespace qtheifimageplugin
