#ifndef QTHEIFIMAGEPLUGIN_IO_HANDLER_H_
#define QTHEIFIMAGEPLUGIN_IO_HANDLER_H_

#include <libheif/heif_cxx.h>

#include <QIODevice>
#include <QImageIOHandler>
#include <QSize>

#include <memory>

namespace qtheifimageplugin {

class IOHandler : public QImageIOHandler
{
 public:
  enum class Format
  {
    none,
    heif,
    heifSequence,
    heic,
    heicSequence,
  };

  explicit IOHandler();
  virtual ~IOHandler();

  bool canRead() const override;
  bool read(QImage* image) override;

  bool write(const QImage& image) override;

  QVariant option(ImageOption option) const override;
  void setOption(ImageOption option, const QVariant& value) override;
  bool supportsOption(ImageOption option) const override;

  static Format canReadFrom(QIODevice& device);

 private:
  struct ReadState
  {
    const QByteArray fileData;
    heif::Context context{};
    heif::ImageHandle handle{};
    heif::Image image{};
    QSize size{};
  };

  IOHandler(const IOHandler& handler) = delete;
  IOHandler& operator=(const IOHandler& handler) = delete;

  /**
   * Updates device and associated state upon device change.
   */
  void updateDevice();

  /**
   * Reads image data from device.
   * Throws heif::Error.
   */
  void loadContext();

  //
  // Private data
  //

  QIODevice* _device = nullptr;

  std::unique_ptr<ReadState> _readState;

  int _quality;
};

}  // namespace qtheifimageplugin

#endif  // QTHEIFIMAGEPLUGIN_IO_HANDLER_H_
