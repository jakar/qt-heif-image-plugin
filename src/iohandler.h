#ifndef HEIFIMAGEPLUGIN_IOHANDLER_H_
#define HEIFIMAGEPLUGIN_IOHANDLER_H_

#include "logging.h"

#include <QIODevice>
#include <QImageIOHandler>

#include <memory>

namespace heif { class Context; }

namespace heifimageplugin
{
  class IOHandler : public QImageIOHandler
  {
   public:
    explicit IOHandler(log::LoggerPtr log_);
    virtual ~IOHandler();

    bool canRead() const override;
    bool read(QImage* image) override;

    QVariant option(ImageOption option_) const override;
    void setOption(ImageOption option_, QVariant const& value) override;
    bool supportsOption(ImageOption option_) const override;

    static bool canReadFrom(QIODevice& device, log::LoggerPtr log);

   private:
    IOHandler(IOHandler const&) = delete;
    IOHandler& operator=(IOHandler const&) = delete;

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

    log::LoggerPtr _log;
    QIODevice* _device = nullptr;
    std::unique_ptr<heif::Context> _context;
  };
}

#endif // HEIFIMAGEPLUGIN_IOHANDLER_H_
