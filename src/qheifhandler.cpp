/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the WebP plugins in the Qt ImageFormats module.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qheifhandler_p.h"

#include "contextwriter_p.h"

#include <libheif/heif_cxx.h>

#include <QtGui/QImage>
#include <QtCore/QSize>
#include <QtCore/QVariant>

#include <algorithm>
#include <cstdint>
#include <memory>

constexpr int kDefaultQuality = 50;  // TODO: maybe adjust this

QHeifHandler::QHeifHandler() :
    QImageIOHandler(),
    _device{nullptr},
    _readState{nullptr},
    _quality{kDefaultQuality}
{
    qDebug("QHeifHandler!");
}

QHeifHandler::~QHeifHandler()
{
}

void QHeifHandler::updateDevice()
{
    // !_device ==> !_readState
    Q_ASSERT(_device || !_readState);

    if (!device()) {
        qWarning("QHeifHandler::updateDevice() device is null");
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

QHeifHandler::Format QHeifHandler::canReadFrom(QIODevice& device)
{
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

bool QHeifHandler::canRead() const
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

QHeifHandler::ReadState::ReadState(QByteArray&& data,
                                heif::Context&& ctx,
                                std::vector<heif_item_id>&& ids,
                                int index) :
    fileData(std::move(data)),
    context(std::move(ctx)),
    idList(std::move(ids)),
    currentIndex(index)
{
}

void QHeifHandler::loadContext()
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
        qDebug("QHeifHandler::loadContext() failed to read file data");
        return;
    }

    // set up new context
    auto context = readContextFromMemory(fileData.constData(), fileData.size());
    auto idList = context.get_list_of_top_level_image_IDs();
    int numImages = context.get_number_of_top_level_images();

    if (numImages < 0 || static_cast<size_t>(numImages) != idList.size()) {
        qWarning("QHeifHandler::loadContext() id list size (%lu) does not match number of images (%d)", idList.size(), numImages);
        return;
    }

    // find primary image in sequence; no ordering guaranteed for id values
    auto id = context.get_primary_image_ID();
    auto iter = std::find(idList.begin(), idList.end(), id);

    if (iter == idList.end()) {
        qDebug("QHeifHandler::loadContext() primary image not found in id list");
        return;
    }

    int currentIndex = static_cast<int>(iter - idList.begin());

    _readState.reset(new ReadState{std::move(fileData),
                                   std::move(context),
                                   std::move(idList),
                                   currentIndex});
}

bool QHeifHandler::read(QImage* destImage)
{
    if (!destImage) {
        qWarning("QHeifHandler::read() QImage to read into is null");
        return false;
    }

#ifndef QT_NO_EXCEPTIONS
    try {
#endif
        loadContext();

        if (!_readState) {
            qWarning("QHeifHandler::read() failed to create context");
            return false;
        }

        auto id = _readState->idList.at(_readState->currentIndex);

        auto handle = _readState->context.get_image_handle(id);
        auto srcImage = handle.decode_image(heif_colorspace_RGB,
                                            heif_chroma_interleaved_RGBA);

        auto channel = heif_channel_interleaved;
        QSize imgSize(srcImage.get_width(channel),
                      srcImage.get_height(channel));

        if (!imgSize.isValid()) {
            qWarning("QHeifHandler::read() invalid image size: %d x %d", imgSize.width(), imgSize.height());
            return false;
        }

        int stride = 0;
        const uint8_t* data = srcImage.get_plane(channel, &stride);

        if (!data) {
            qWarning("QHeifHandler::read() pixel data not found");
            return false;
        }

        if (stride <= 0) {
            qWarning("QHeifHandler::read() invalid stride: %d", stride);
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
#ifndef QT_NO_EXCEPTIONS
    } catch (const heif::Error& error) {
        qWarning("QHeifHandler::write() libheif read error: %s", error.get_message().c_str());
    }
#endif
    return false;
}

int QHeifHandler::currentImageNumber() const
{
    if (!_readState) {
        return -1;
    }

    return _readState->currentIndex;
}

int QHeifHandler::imageCount() const
{
    if (!_readState) {
        return 0;
    }

    return _readState->context.get_number_of_top_level_images();
}

bool QHeifHandler::jumpToImage(int index)
{
    if (!_readState) {
        return false;
    }

    if (index < 0 || static_cast<size_t>(index) >= _readState->idList.size()) {
        return false;
    }

    _readState->currentIndex = index;
    return true;
}

bool QHeifHandler::jumpToNextImage()
{
    if (!_readState) {
        return false;
    }

    return jumpToImage(_readState->currentIndex + 1);
}

bool QHeifHandler::write(const QImage& preConvSrcImage)
{
    updateDevice();

    if (!device()) {
        qWarning("QHeifHandler::write() device null before write");
        return false;
    }

    if (preConvSrcImage.isNull()) {
        qWarning("QHeifHandler::write() source image is null");
        return false;
    }

    const QImage srcImage =
        preConvSrcImage.convertToFormat(QImage::Format_RGBA8888);

    const QSize size = srcImage.size();

    if (srcImage.isNull() || !size.isValid()) {
        qWarning("QHeifHandler::write() source image format conversion failed");
        return false;
    }

#ifndef QT_NO_EXCEPTIONS
    try {
#endif
        heif::Image destImage{};
        destImage.create(size.width(), size.height(),
                         heif_colorspace_RGB,
                         heif_chroma_interleaved_RGBA);

        auto channel = heif_channel_interleaved;
        destImage.add_plane(channel, size.width(), size.height(), 32);

        int destStride = 0;
        uint8_t* destData = destImage.get_plane(channel, &destStride);

        if (!destData) {
            qWarning("QHeifHandler::write() could not get libheif image plane");
            return false;
        }

        if (destStride <= 0) {
            qWarning("QHeifHandler::write() invalid destination stride: %d", destStride);
            return false;
        }

        const uint8_t* srcData = srcImage.constBits();
        const int srcStride = srcImage.bytesPerLine();

        if (!srcData) {
            qWarning("QHeifHandler::write() source image data is null");
            return false;
        }

        if (srcStride <= 0) {
            qWarning("QHeifHandler::write() invalid source image stride: %d", srcStride);
            return false;
        } else if (srcStride > destStride) {
            qWarning("QHeifHandler::write() source line larger than destination");
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

#ifndef QT_NO_EXCEPTIONS
    } catch (const heif::Error& error) {
        qWarning("QHeifHandler::write() libheif write error: %s", error.get_message().c_str());
    }
#endif

    return false;
}

QVariant QHeifHandler::option(ImageOption opt) const
{
    Q_UNUSED(opt);
    return {};
}

void QHeifHandler::setOption(ImageOption opt, const QVariant& value)
{
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

bool QHeifHandler::supportsOption(ImageOption opt) const
{
    return opt == Quality;
}
