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

#include <qimageiohandler.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_WEBP
#undef QT_NO_IMAGEFORMAT_WEBP
#endif
#include "qheifhandler_p.h"

#include <qiodevice.h>
#include <qbytearray.h>

QT_BEGIN_NAMESPACE

class QHeifPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "heif.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QHeifPlugin::Capabilities QHeifPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    const bool formatOK = (format == "heic" || format == "heics"
                           || format == "heif" || format == "heifs");

    if (!formatOK && !format.isEmpty()) {
        return {};
    }

    if (!device) {
        if (formatOK) {
            return CanRead | CanWrite;
        } else {
            return {};
        }
    }

    using F = QHeifHandler::Format;
    Capabilities caps{};

    if (device->isReadable() && QHeifHandler::canReadFrom(*device) != F::None) {
        caps |= CanRead;
    }

    if (device->isWritable()) {
        caps |= CanWrite;
    }

    return caps;
}

QImageIOHandler *QHeifPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QHeifHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE

#include "main.moc"

#endif // !QT_NO_IMAGEFORMATPLUGIN
