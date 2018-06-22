#ifndef QTHEIFIMAGEPLUGIN_PLUGIN_H_
#define QTHEIFIMAGEPLUGIN_PLUGIN_H_

#include <QImageIOPlugin>

namespace qtheifimageplugin {

class Plugin : public QImageIOPlugin
{
  Q_OBJECT;
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface"
                    FILE "heif.json");

 public:
  Plugin(QObject* parent_ = nullptr);
  virtual ~Plugin();

  Capabilities capabilities(QIODevice* device,
                            const QByteArray& format) const override;

  QImageIOHandler* create(QIODevice* device,
                          const QByteArray& format = QByteArray()) const override;
};

}  // namespace qtheifimageplugin

#endif  // QTHEIFIMAGEPLUGIN_PLUGIN_H_
