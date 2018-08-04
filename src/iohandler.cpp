#include "iohandler.h"

#include "contextwriter.h"
#include "log.h"

#include <libheif/heif_cxx.h>

#include <QImage>
#include <QSize>
#include <QVariant>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace {

constexpr int kDefaultQuality = 50;  // TODO: maybe adjust this

}  // namespace

namespace qtheifimageplugin {

IOHandler::IOHandler() :
    QImageIOHandler(),
    _device{nullptr},
    _readState{nullptr},
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
    // !_device ==> !_readState
    Q_ASSERT(_device || !_readState);

    if (!device()) {
        log::warning() << "device is null";
    }

    if (device() != _device) {
        // new device; re-read data
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

    if (header.size() != kHeaderSize) {
        return Format::None;
    }

    // skip first four bytes, which contain box size
    const QByteArray w1 = header.mid(4, 4);
    const QByteArray w2 = header.mid(8, 4);

    if (w1 != "ftyp") {
        // not an ftyp box
        return Format::None;
    }

    // brand follows box name, determines format
    QTHEIFIMAGEPLUGIN_LOG_TRACE("ftyp: " << w2);

    if (w2 == "mif1") {
        return Format::Heif;
    } else if (w2 == "msf1") {
        return Format::HeifSequence;
    } else if (w2 == "heic" || w2 == "heix") {
        return Format::Heic;
    } else if (w2 == "hevc" || w2 == "hevx") {
        return Format::HeicSequence;
    } else {
        return Format::None;
    }
}

bool IOHandler::canRead() const
{
    if (!device()) {
        return false;
    }

    auto mimeFormat = canReadFrom(*device());

    // Other image plugins set the format here. Not sure if it is really
    // necessary or what it accomplishes.
    switch (mimeFormat) {
    case Format::Heif:
        setFormat("heif");
        return true;

    case Format::HeifSequence:
        setFormat("heifs");
        return true;

    case Format::Heic:
        setFormat("heic");
        return true;

    case Format::HeicSequence:
        setFormat("heics");
        return true;

    default:
        return false;
    }
}

//
// Reading
//

namespace {

heif::Context readContextFromMemory(const void* mem, size_t size)
{
    heif::Context context{};

#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
    context.read_from_memory_without_copy(mem, size);
#else
    context.read_from_memory(mem, size);
#endif

    return context;
}

}  // namespace

IOHandler::ReadState::ReadState(QByteArray&& data,
                                heif::Context&& ctx,
                                std::vector<heif_item_id>&& ids,
                                int index) :
    fileData(std::move(data)),
    context(std::move(ctx)),
    idList(std::move(ids)),
    currentIndex(index)
{
}

void IOHandler::loadContext()
{
    updateDevice();

    if (!device()) {
        return;
    }

    if (_readState) {
        // context already loaded
        return;
    }

    // read file
    auto fileData = device()->readAll();

    if (fileData.isEmpty()) {
        log::debug() << "failed to read file data";
        return;
    }

    // set up new context
    auto context = readContextFromMemory(fileData.constData(), fileData.size());
    auto idList = context.get_list_of_top_level_image_IDs();
    int numImages = context.get_number_of_top_level_images();

    if (numImages < 0 || static_cast<size_t>(numImages) != idList.size()) {
        log::warning()
            << "id list size (" << idList.size()
            << ") does not match number of images (" << numImages << ")";
        return;
    }

    // find primary image in sequence; no ordering guaranteed for id values
    auto id = context.get_primary_image_ID();
    auto iter = std::find(idList.begin(), idList.end(), id);

    if (iter == idList.end()) {
        log::debug() << "primary image not found in id list";
        return;
    }

    int currentIndex = static_cast<int>(iter - idList.begin());

    QTHEIFIMAGEPLUGIN_LOG_TRACE("num images: " << idList.size()
                                << ", primary id: " << id
                                << ", index: " << currentIndex);

    _readState.reset(new ReadState{std::move(fileData),
                                   std::move(context),
                                   std::move(idList),
                                   currentIndex});
}

bool IOHandler::read(QImage* destImage)
{
    QTHEIFIMAGEPLUGIN_LOG_TRACE("");

    if (!destImage) {
        log::warning() << "QImage to read into is null";
        return false;
    }

    try {
        loadContext();

        if (!_readState) {
            log::debug() << "failed to create context";
            return false;
        }

        auto id = _readState->idList.at(_readState->currentIndex);

        QTHEIFIMAGEPLUGIN_LOG_TRACE("id: " << id
                                    << ", index: " << _readState->currentIndex);

        auto handle = _readState->context.get_image_handle(id);
        auto srcImage = handle.decode_image(heif_colorspace_RGB,
                                            heif_chroma_interleaved_RGBA);

        auto channel = heif_channel_interleaved;
        QSize imgSize(srcImage.get_width(channel),
                      srcImage.get_height(channel));

        if (!imgSize.isValid()) {
            log::debug() << "invalid image size: "
                << imgSize.width() << "x" << imgSize.height();
            return false;
        }

        int stride = 0;
        const uint8_t* data = srcImage.get_plane(channel, &stride);

        if (!data) {
            log::warning() << "pixel data not found";
            return false;
        }

        if (stride <= 0) {
            log::warning() << "invalid stride: " << stride;
            return false;
        }

        // image object copy/move refers to same data
        auto* dataImage = new heif::Image(std::move(srcImage));

        *destImage = QImage(
            data, imgSize.width(), imgSize.height(),
            stride, QImage::Format_RGBA8888,
            [](void* d) { delete static_cast<heif::Image*>(d); },
            dataImage
        );

        return true;

    } catch (const heif::Error& error) {
        log::warning() << "libheif read error: " << error.get_message().c_str();
    }

    return false;
}

//
// Jumping
//

int IOHandler::currentImageNumber() const
{
    if (!_readState) {
        return -1;
    }

    return _readState->currentIndex;
}

int IOHandler::imageCount() const
{
    if (!_readState) {
        return 0;
    }

    return _readState->context.get_number_of_top_level_images();
}

bool IOHandler::jumpToImage(int index)
{
    QTHEIFIMAGEPLUGIN_LOG_TRACE("index: " << index);

    if (!_readState) {
        return false;
    }

    if (index < 0 || static_cast<size_t>(index) >= _readState->idList.size()) {
        return false;
    }

    _readState->currentIndex = index;
    return true;
}

bool IOHandler::jumpToNextImage()
{
    if (!_readState) {
        return false;
    }

    return jumpToImage(_readState->currentIndex + 1);
}

//
// Writing
//

bool IOHandler::write(const QImage& preConvSrcImage)
{
    QTHEIFIMAGEPLUGIN_LOG_TRACE("");

    updateDevice();

    if (!device()) {
        log::warning() << "device null before write";
        return false;
    }

    if (preConvSrcImage.isNull()) {
        log::warning() << "source image is null";
        return false;
    }

    const QImage srcImage =
        preConvSrcImage.convertToFormat(QImage::Format_RGBA8888);

    const QSize size = srcImage.size();

    if (srcImage.isNull() || !size.isValid()) {
        log::warning() << "source image format conversion failed";
        return false;
    }

    try {
        heif::Image destImage{};
        destImage.create(size.width(), size.height(),
                         heif_colorspace_RGB,
                         heif_chroma_interleaved_RGBA);

        auto channel = heif_channel_interleaved;
        destImage.add_plane(channel, size.width(), size.height(), 32);

        int destStride = 0;
        uint8_t* destData = destImage.get_plane(channel, &destStride);

        if (!destData) {
            log::warning() << "could not get libheif image plane";
            return false;
        }

        if (destStride <= 0) {
            log::warning() << "invalid destination stride: " << destStride;
            return false;
        }

        const uint8_t* srcData = srcImage.constBits();
        const int srcStride = srcImage.bytesPerLine();

        if (!srcData) {
            log::warning() << "source image data is null";
            return false;
        }

        if (srcStride <= 0) {
            log::warning() << "invalid source image stride: " << srcStride;
            return false;
        } else if (srcStride > destStride) {
            log::warning() << "source line larger than destination";
            return false;
        }

        // copy rgba data
        for (int y = 0; y < size.height(); ++y) {
            auto* srcBegin = srcData + y * srcStride;
            auto* srcEnd = srcBegin + srcStride;
            std::copy(srcBegin, srcEnd, destData + y * destStride);
        }

        // encode and write
        heif::Encoder encoder(heif_compression_HEVC);
        encoder.set_lossy_quality(_quality);

        heif::Context context{};
        context.encode_image(destImage, encoder);

        ContextWriter writer(*device());
        context.write(writer);

        return true;

    } catch (const heif::Error& error) {
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

    Q_UNUSED(opt);
    return {};
}

void IOHandler::setOption(ImageOption opt, const QVariant& value)
{
    QTHEIFIMAGEPLUGIN_LOG_TRACE("opt: " << opt << ", value: " << value);

    switch (opt) {
    case Quality: {
        bool ok = false;
        int q = value.toInt(&ok);

        if (ok && q >= 0 && q <= 100) {
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

    return opt == Quality;
}

}  // namespace qtheifimageplugin
