#ifndef HEIF_IMAGE_PLUGIN_PLUGIN_H_
#define HEIF_IMAGE_PLUGIN_PLUGIN_H_

#include <QImageIOPlugin>

namespace heif_image_plugin {

class Plugin : public QImageIOPlugin {
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

 private:
};

}  // namespace heif_image_plugin

#endif  // HEIF_IMAGE_PLUGIN_PLUGIN_H_
