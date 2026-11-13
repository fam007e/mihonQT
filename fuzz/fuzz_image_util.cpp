#include <stdint.h>
#include <stddef.h>
#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include "../src/util/ImageUtil.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0 || size > 10 * 1024 * 1024) { // Cap at 10MB
        return 0;
    }

    QByteArray raw(reinterpret_cast<const char*>(data), static_cast<int>(size));
    
    // Test QImage decoding (which triggers WebP fallback and Qt image decoders)
    QImage img = ImageUtil::loadFromData(raw);
    if (!img.isNull()) {
        // Exercise basic image properties to ensure buffer is valid and readable
        volatile int w = img.width();
        volatile int h = img.height();
        volatile int bpl = img.bytesPerLine();
        const uchar* bits = img.constBits();
        if (bits && w > 0 && h > 0) {
            volatile uchar firstByte = bits[0];
            volatile uchar lastByte = bits[img.sizeInBytes() - 1];
            (void)firstByte;
            (void)lastByte;
        }
        (void)w;
        (void)h;
        (void)bpl;
    }

    return 0;
}
