
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    for (int i = 1; i < argc; i++) {
        const QString imgPath(argv[i]);
        QImage image(imgPath);
        if (!image.isNull()) {
            qDebug() << "image size:" << image.size();
            QLabel* label = new QLabel();
            int w = 0;
            int h = 0;
            if (image.width() > image.height()) {
                w = 1400;
                h = (1.0 * image.height()) / (1.0 * image.width() * 1400);
            } else {
                h = 1400;
                w = (1.0 * 1400 * image.height()) /(1.0 * image.width());
            }
            qDebug() << w << h;
            const QPixmap pixmap = QPixmap::fromImage(image);
            label->setPixmap(pixmap);
            label->show();
            label->resize(image.size());
        } else {
            qCritical() << "Invalid image:" << imgPath;
        }
    }

    return a.exec();
}
