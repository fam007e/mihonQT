#include <QTest>
#include <QObject>
#include <QBuffer>
#include "../src/util/ImageUtil.h"

class TestImageUtil : public QObject
{
    Q_OBJECT

private slots:
    void testEmptyAndCorruptBuffers();
    void testTruncatedHeaders();
    void testValidPngRoundtrip();
    void testMalformedWebpHeader();
};

void TestImageUtil::testEmptyAndCorruptBuffers()
{
    // Empty data
    QByteArray empty;
    QImage img1 = ImageUtil::loadFromData(empty);
    QVERIFY(img1.isNull());
    QPixmap pix1 = ImageUtil::loadPixmapFromData(empty);
    QVERIFY(pix1.isNull());

    // Random garbage data
    QByteArray garbage("NOT_AN_IMAGE_DATA_AT_ALL_1234567890!@#$%^&*()");
    QImage img2 = ImageUtil::loadFromData(garbage);
    QVERIFY(img2.isNull());
}

void TestImageUtil::testTruncatedHeaders()
{
    // 1-byte, 2-byte, 3-byte, 4-byte prefixes of common image formats
    const char pngSig[] = "\x89PNG\r\n\x1a\n";
    for (int len = 1; len <= 8; ++len) {
        QByteArray truncated(pngSig, len);
        QImage img = ImageUtil::loadFromData(truncated);
        QVERIFY(img.isNull());
    }

    const char webpSig[] = "RIFF\x00\x00\x00\x00WEBP";
    for (int len = 1; len <= 12; ++len) {
        QByteArray truncated(webpSig, len);
        QImage img = ImageUtil::loadFromData(truncated);
        QVERIFY(img.isNull());
    }
}

void TestImageUtil::testValidPngRoundtrip()
{
    // Create a known valid 32x32 image and serialize to PNG
    QImage testImg(32, 32, QImage::Format_RGBA8888);
    testImg.fill(QColor(255, 0, 0, 255));

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    QVERIFY(testImg.save(&buffer, "PNG"));
    buffer.close();

    QImage decoded = ImageUtil::loadFromData(bytes);
    QVERIFY(!decoded.isNull());
    QCOMPARE(decoded.width(), 32);
    QCOMPARE(decoded.height(), 32);

    QPixmap pixmap = ImageUtil::loadPixmapFromData(bytes);
    QVERIFY(!pixmap.isNull());
    QCOMPARE(pixmap.width(), 32);
}

void TestImageUtil::testMalformedWebpHeader()
{
    // Header claims to be RIFF....WEBP but payload is invalid / out of bounds
    QByteArray malformed;
    malformed.append("RIFF", 4);
    uint32_t fakeSize = 100000;
    malformed.append(reinterpret_cast<const char*>(&fakeSize), 4);
    malformed.append("WEBPVP8 ", 8);
    // add a few junk bytes
    malformed.append("junkjunkjunk", 12);

    QImage img = ImageUtil::loadFromData(malformed);
    // Should safely reject without buffer overflow or crash
    QVERIFY(img.isNull());
}

QTEST_MAIN(TestImageUtil)
#include "test_image_util.moc"
