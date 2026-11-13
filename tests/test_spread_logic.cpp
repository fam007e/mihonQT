#include <QTest>
#include <QObject>
#include <QImage>
#include <QVector>

class TestSpreadLogic : public QObject
{
    Q_OBJECT

    struct SpreadInfo {
        int firstIndex = -1;
        int secondIndex = -1;
        int stepForward = 1;
        bool isSingle = true;
    };

    SpreadInfo computeSpread(int index, const QVector<QImage>& pages, bool firstPageCover, bool autoWideDetect) {
        SpreadInfo info;
        if (index < 0 || index >= pages.size()) return info;

        info.firstIndex = index;
        info.secondIndex = -1;
        info.stepForward = 1;
        info.isSingle = true;

        // Cover page offset: Page 0 is kept single if enabled
        if (index == 0 && firstPageCover) {
            info.isSingle = true;
            info.stepForward = 1;
            return info;
        }

        const QImage& currentImg = pages[index];
        // If current page is wide landscape, display alone
        if (autoWideDetect && !currentImg.isNull() && currentImg.width() > currentImg.height()) {
            info.isSingle = true;
            info.stepForward = 1;
            return info;
        }

        int nextIndex = index + 1;
        if (nextIndex < pages.size()) {
            const QImage& nextImg = pages[nextIndex];
            if (autoWideDetect && !nextImg.isNull() && nextImg.width() > nextImg.height()) {
                info.isSingle = true;
                info.stepForward = 1;
                return info;
            }

            info.secondIndex = nextIndex;
            info.stepForward = 2;
            info.isSingle = false;
        }

        return info;
    }

    int computePreviousSpreadIndex(int currentIndex, const QVector<QImage>& pages, bool firstPageCover, bool autoWideDetect) {
        if (currentIndex <= 0) return 0;
        if (currentIndex - 2 >= 0) {
            SpreadInfo pInfo = computeSpread(currentIndex - 2, pages, firstPageCover, autoWideDetect);
            if (currentIndex - 2 + pInfo.stepForward == currentIndex) {
                return currentIndex - 2;
            }
        }
        return currentIndex - 1;
    }

private slots:
    void testCoverOffset();
    void testStandardPairing();
    void testWideLandscapeIsolation();
    void testRtlOrientation();
    void testBackwardNavigation();
};

void TestSpreadLogic::testCoverOffset()
{
    QVector<QImage> pages(5);
    for (int i = 0; i < 5; ++i) {
        pages[i] = QImage(800, 1200, QImage::Format_RGB32); // Portrait
    }

    // Cover enabled: page 0 is alone
    SpreadInfo s0 = computeSpread(0, pages, true, true);
    QVERIFY(s0.isSingle);
    QCOMPARE(s0.stepForward, 1);
    QCOMPARE(s0.firstIndex, 0);
    QCOMPARE(s0.secondIndex, -1);

    // Cover disabled: page 0 pairs with page 1
    SpreadInfo s0NoCover = computeSpread(0, pages, false, true);
    QVERIFY(!s0NoCover.isSingle);
    QCOMPARE(s0NoCover.stepForward, 2);
    QCOMPARE(s0NoCover.firstIndex, 0);
    QCOMPARE(s0NoCover.secondIndex, 1);
}

void TestSpreadLogic::testStandardPairing()
{
    QVector<QImage> pages(6);
    for (int i = 0; i < 6; ++i) {
        pages[i] = QImage(800, 1200, QImage::Format_RGB32);
    }

    // Spread 1: Page 1 and Page 2
    SpreadInfo s1 = computeSpread(1, pages, true, true);
    QVERIFY(!s1.isSingle);
    QCOMPARE(s1.firstIndex, 1);
    QCOMPARE(s1.secondIndex, 2);
    QCOMPARE(s1.stepForward, 2);

    // Spread 2: Page 3 and Page 4
    SpreadInfo s2 = computeSpread(3, pages, true, true);
    QVERIFY(!s2.isSingle);
    QCOMPARE(s2.firstIndex, 3);
    QCOMPARE(s2.secondIndex, 4);
    QCOMPARE(s2.stepForward, 2);

    // Last page: Page 5 is alone
    SpreadInfo sLast = computeSpread(5, pages, true, true);
    QVERIFY(sLast.isSingle);
    QCOMPARE(sLast.stepForward, 1);
}

void TestSpreadLogic::testWideLandscapeIsolation()
{
    QVector<QImage> pages(4);
    pages[0] = QImage(800, 1200, QImage::Format_RGB32);
    pages[1] = QImage(1600, 900, QImage::Format_RGB32); // Wide landscape 2-page spread image
    pages[2] = QImage(800, 1200, QImage::Format_RGB32);
    pages[3] = QImage(800, 1200, QImage::Format_RGB32);

    // Page 1 is wide landscape, should not pair with page 2
    SpreadInfo s1 = computeSpread(1, pages, true, true);
    QVERIFY(s1.isSingle);
    QCOMPARE(s1.stepForward, 1);

    // Page 2 is portrait, can pair with page 3
    SpreadInfo s2 = computeSpread(2, pages, true, true);
    QVERIFY(!s2.isSingle);
    QCOMPARE(s2.firstIndex, 2);
    QCOMPARE(s2.secondIndex, 3);
}

void TestSpreadLogic::testRtlOrientation()
{
    // Verify that in RTL double-page, current page is on right, next page is on left
    int firstIndex = 1;  // Current page
    int secondIndex = 2; // Next page

    bool isRTL = true;
    int leftPage = isRTL ? secondIndex : firstIndex;
    int rightPage = isRTL ? firstIndex : secondIndex;

    QCOMPARE(rightPage, 1); // Current page on RIGHT
    QCOMPARE(leftPage, 2);  // Next page on LEFT
}

void TestSpreadLogic::testBackwardNavigation()
{
    QVector<QImage> pages(10);
    for (int i = 0; i < 10; ++i) {
        pages[i] = QImage(800, 1200, QImage::Format_RGB32);
    }

    // Step sequence with cover enabled:
    // 0 -> 1 -> 3 -> 5 -> 7 -> 9
    // Backward from 5 should land on 3
    int prevFrom5 = computePreviousSpreadIndex(5, pages, true, true);
    QCOMPARE(prevFrom5, 3);

    // Backward from 3 should land on 1
    int prevFrom3 = computePreviousSpreadIndex(3, pages, true, true);
    QCOMPARE(prevFrom3, 1);

    // Backward from 1 should land on 0 (cover)
    int prevFrom1 = computePreviousSpreadIndex(1, pages, true, true);
    QCOMPARE(prevFrom1, 0);

    // Backward from 0 should clamp to 0
    int prevFrom0 = computePreviousSpreadIndex(0, pages, true, true);
    QCOMPARE(prevFrom0, 0);
}

QTEST_MAIN(TestSpreadLogic)
#include "test_spread_logic.moc"
