#include "ImageUtil.h"
#include <webp/decode.h>
#include <QDebug>

QImage ImageUtil::loadFromData(const QByteArray& data)
{
    if (data.isEmpty()) return QImage();

    QImage img;
    if (img.loadFromData(data)) {
        return img;
    }

    // Fallback: try WebP decoding via libwebp
    int width = 0, height = 0;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.constData());
    if (WebPGetInfo(bytes, data.size(), &width, &height)) {
        QImage webpImg(width, height, QImage::Format_RGBA8888);
        if (WebPDecodeRGBAInto(bytes, data.size(), webpImg.bits(), webpImg.sizeInBytes(), webpImg.bytesPerLine())) {
            return webpImg;
        }
    }

    return QImage();
}

QPixmap ImageUtil::loadPixmapFromData(const QByteArray& data)
{
    QImage img = loadFromData(data);
    if (!img.isNull()) {
        return QPixmap::fromImage(img);
    }
    return QPixmap();
}
