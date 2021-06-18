/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the WebP plugins in the Qt ImageFormats module.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QHEIFHANDLER_P_H
#define QHEIFHANDLER_P_H

#include <libheif/heif.h>

#include <QtCore/QIODevice>
#include <QtGui/QImageIOHandler>

#include <memory>
#include <vector>

class QHeifHandler : public QImageIOHandler
{
public:
    enum class Format
    {
        None,
        Heif,
        HeifSequence,
        Heic,
        HeicSequence,
        Avif,
    };

    explicit QHeifHandler();
    virtual ~QHeifHandler();

    QHeifHandler(const QHeifHandler& handler) = delete;
    QHeifHandler& operator=(const QHeifHandler& handler) = delete;

    bool canRead() const override;
    bool read(QImage* image) override;

    bool write(const QImage& image) override;

    int currentImageNumber() const override;
    int imageCount() const override;
    bool jumpToImage(int index) override;
    bool jumpToNextImage() override;

    QVariant option(ImageOption opt) const override;
    void setOption(ImageOption opt, const QVariant& value) override;
    bool supportsOption(ImageOption opt) const override;

    static Format canReadFrom(QIODevice& device);

private:
    struct ReadState
    {
        ReadState(QByteArray&& data,
                  std::shared_ptr<heif_context>&& ctx,
                  std::vector<heif_item_id>&& ids,
                  int index);

        const QByteArray fileData;
        const std::shared_ptr<heif_context> context;
        const std::vector<heif_item_id> idList;
        int currentIndex{};
    };

    /**
     * Updates device and associated state upon device change.
     */
    void updateDevice();

    /**
     * Reads data from device. Creates read state.
     */
    void loadContext();

    //
    // Private data
    //

    QIODevice* _device = nullptr;

    std::unique_ptr<ReadState> _readState;  // non-null iff context is loaded

    int _quality;
};

#endif  // QHEIFHANDLER_P_H
