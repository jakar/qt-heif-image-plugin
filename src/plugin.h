#ifndef HEIFIMAGEPLUGIN_PLUGIN_H_
#define HEIFIMAGEPLUGIN_PLUGIN_H_

#include "logging.h"

#include <QImageIOPlugin>

namespace heifimageplugin
{
  class Plugin : public QImageIOPlugin
  {
    Q_OBJECT;
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface"
                      FILE "heif.json");

   public:

    Plugin(QObject* parent_ = nullptr);

    virtual ~Plugin();

    Capabilities capabilities(QIODevice* device,
                              QByteArray const& format) const override;

    QImageIOHandler* create(
      QIODevice* device,
      QByteArray const& format = QByteArray()) const override;

   private:

    log::LoggerPtr _log;
  };
}

#endif // HEIFIMAGEPLUGIN_PLUGIN_H_
