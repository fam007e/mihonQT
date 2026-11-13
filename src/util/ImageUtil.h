#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QImage>
#include <QByteArray>
#include <QPixmap>

class ImageUtil
{
public:
    static QImage loadFromData(const QByteArray& data);
    static QPixmap loadPixmapFromData(const QByteArray& data);
};

#endif // IMAGEUTIL_H
