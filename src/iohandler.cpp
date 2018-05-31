#include "iohandler.h"

#include "assertion.h"
#include "logging.h"

#include <libheif/heif_cxx.h>

#include <QImage>
#include <QSize>
#include <QVariant>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace
{
  using namespace std::literals;
}

namespace heifimageplugin
{
  IOHandler::IOHandler(log::LoggerPtr log_)
  :
    QImageIOHandler(),
    _log(log_)
  {
    HEIFIMAGEPLUGIN_ASSERT(_log);
    HEIFIMAGEPLUGIN_TRACE(_log);
  }

  IOHandler::~IOHandler()
  {
    HEIFIMAGEPLUGIN_TRACE(_log);
  }

  //
  // Peeking
  //

  bool IOHandler::canReadFrom(QIODevice& device,
                              log::LoggerPtr log [[maybe_unused]])
  {
    HEIFIMAGEPLUGIN_ASSERT(log);
    HEIFIMAGEPLUGIN_TRACE(log);

    // logic taken from qt macheif plugin
    constexpr int headerSize = 12;
    auto header = device.peek(headerSize);

    if (header.size() != headerSize)
    {
      HEIFIMAGEPLUGIN_DEBUG(log, "could not read header");
      return false;
    }

    std::string_view hv(header.data(), headerSize);
    auto w1 = hv.substr(4, 4);
    auto w2 = hv.substr(8, 4);

    HEIFIMAGEPLUGIN_DEBUG(log, "header data: {}", hv);

    return
      w1 == "ftyp"sv
      && (w2 == "heic"sv || w2 == "heix"sv || w2 == "mifi"sv);
  }

  bool IOHandler::canRead() const
  {
    if (device() && canReadFrom(*device(), _log))
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
      HEIFIMAGEPLUGIN_WARN(_log, "device is null");
      HEIFIMAGEPLUGIN_ASSERT(_context == nullptr);
    }

    if (device() != _device)
    {
      HEIFIMAGEPLUGIN_TRACE(_log, "updating device...");
      _device = device();
      _context.reset();
    }
  }

  void IOHandler::loadContext()
  {
    HEIFIMAGEPLUGIN_TRACE(_log);
    updateDevice();

    if (_context || !device())
      return;

    HEIFIMAGEPLUGIN_DEBUG(_log, "loading image data...");
    auto fileData = device()->readAll();

    if (fileData.isEmpty())
    {
      HEIFIMAGEPLUGIN_ERROR(_log, "failed to read file data");
      return;
    }

    auto context = std::make_unique<heif::Context>();
    context->read_from_memory(fileData.data(), fileData.size());

    _context = std::move(context);
  }

  bool IOHandler::read(QImage* qimage)
  {
    HEIFIMAGEPLUGIN_TRACE(_log);

    if (!qimage)
    {
      HEIFIMAGEPLUGIN_WARN(_log, "image is null");
      return false;
    }

    try
    {
      loadContext();

      if (!_context)
      {
        HEIFIMAGEPLUGIN_DEBUG(_log, "null context during read");
        return false;
      }

      auto handle = _context->get_primary_image_handle();
      auto himage = handle.decode_image(heif_colorspace_RGB,
                                        heif_chroma_interleaved_RGBA);

      auto channel = heif_channel_interleaved;
      int width = himage.get_width(channel);
      int height = himage.get_height(channel);
      HEIFIMAGEPLUGIN_DEBUG(_log, "image size: {} x {}", width, height);

      int stride = 0;
      uint8_t const* data = himage.get_plane(channel, &stride);
      HEIFIMAGEPLUGIN_DEBUG(_log, "data stride: {}", stride);

      // copy image data
      int dataSize = height * stride;
      uint8_t* dataCopy = new uint8_t[dataSize];

      if (!dataCopy)
      {
        log::error(_log, "failed to alloc data copy");
        return false;
      }

      std::copy(data, data + dataSize, dataCopy);

      *qimage = QImage(
        dataCopy, width, height, stride, QImage::Format_RGBA8888,
        [](void* d) {
          HEIFIMAGEPLUGIN_DEBUG("deleting image data...");
          delete[] static_cast<uint8_t*>(d);
        }
      );

      return true;
    }
    catch (heif::Error const& error)
    {
      log::error(_log, "libheif read error: {}", error.get_message());
    }

    return false;
  }

  //
  // Options
  //

  QVariant IOHandler::option(ImageOption option_ [[maybe_unused]]) const
  {
    HEIFIMAGEPLUGIN_TRACE(_log, "option_: {}", option_);
    return {};
  }

  void IOHandler::setOption(ImageOption option_ [[maybe_unused]],
                            QVariant const& value [[maybe_unused]])
  {
    HEIFIMAGEPLUGIN_TRACE(_log, "option_: {}, value: {}",
                          option_, value.toString().toStdString());
  }

  bool IOHandler::supportsOption(ImageOption option_ [[maybe_unused]]) const
  {
    HEIFIMAGEPLUGIN_TRACE(_log, "option_: {}", option_);
    return false;
  }
}
