#include <QTest>
#include <QObject>
#include <algorithm>
#include "../src/model/Chapter.h"
#include "../src/model/Manga.h"
#include "../src/model/ChapterSort.h"

class TestChapterSort : public QObject
{
    Q_OBJECT

private slots:
    void testSourceOrderSort();
    void testChapterNumberSort();
    void testUploadDateSort();
    void testAlphabeticalSort();
    void testStrictWeakOrdering();
};

void TestChapterSort::testSourceOrderSort()
{
    Manga manga; // default flags: CHAPTER_SORTING_SOURCE
    
    Chapter cNewest(1, 1, "/ch3", "Chapter 3", "", false, false, 0, 3.0f, 0, 0, 0, 0, 0, 0);
    Chapter cMiddle(2, 1, "/ch2", "Chapter 2", "", false, false, 0, 2.0f, 1, 0, 0, 0, 0, 0);
    Chapter cOldest(3, 1, "/ch1", "Chapter 1", "", false, false, 0, 1.0f, 2, 0, 0, 0, 0, 0);

    QList<Chapter> list = {cMiddle, cOldest, cNewest};

    // UI View: sortDescending = true (Newest first, source_order 0 first)
    auto compDesc = getChapterComparator(manga, /*sortDescending=*/true);
    std::sort(list.begin(), list.end(), compDesc);

    QCOMPARE(list[0].id(), 1); // cNewest (source_order 0)
    QCOMPARE(list[1].id(), 2); // cMiddle (source_order 1)
    QCOMPARE(list[2].id(), 3); // cOldest (source_order 2)

    // Reader View: sortDescending = false (Ascending reading order: Chapter 1 -> Chapter 2 -> Chapter 3)
    auto compAsc = getChapterComparator(manga, /*sortDescending=*/false);
    std::sort(list.begin(), list.end(), compAsc);

    QCOMPARE(list[0].id(), 3); // cOldest
    QCOMPARE(list[1].id(), 2); // cMiddle
    QCOMPARE(list[2].id(), 1); // cNewest
}

void TestChapterSort::testChapterNumberSort()
{
    Manga manga;
    manga.setChapterFlags(Manga::CHAPTER_SORTING_NUMBER);

    Chapter c1(1, 1, "/1", "Ch 1", "", false, false, 0, 1.0f, 2, 0, 0, 0, 0, 0);
    Chapter c1_5(2, 1, "/1.5", "Ch 1.5", "", false, false, 0, 1.5f, 1, 0, 0, 0, 0, 0);
    Chapter c2(3, 1, "/2", "Ch 2", "", false, false, 0, 2.0f, 0, 0, 0, 0, 0, 0);

    QList<Chapter> list = {c2, c1, c1_5};

    auto compAsc = getChapterComparator(manga, /*sortDescending=*/false);
    std::sort(list.begin(), list.end(), compAsc);

    QCOMPARE(list[0].chapterNumber(), 1.0f);
    QCOMPARE(list[1].chapterNumber(), 1.5f);
    QCOMPARE(list[2].chapterNumber(), 2.0f);

    auto compDesc = getChapterComparator(manga, /*sortDescending=*/true);
    std::sort(list.begin(), list.end(), compDesc);

    QCOMPARE(list[0].chapterNumber(), 2.0f);
    QCOMPARE(list[1].chapterNumber(), 1.5f);
    QCOMPARE(list[2].chapterNumber(), 1.0f);
}

void TestChapterSort::testUploadDateSort()
{
    Manga manga;
    manga.setChapterFlags(Manga::CHAPTER_SORTING_UPLOAD_DATE);

    Chapter cOld(1, 1, "/1", "Ch 1", "", false, false, 0, 1.0f, 0, 0, 1000, 0, 0, 0);
    Chapter cNew(2, 1, "/2", "Ch 2", "", false, false, 0, 2.0f, 0, 0, 5000, 0, 0, 0);

    QList<Chapter> list = {cOld, cNew};
    auto compDesc = getChapterComparator(manga, /*sortDescending=*/true);
    std::sort(list.begin(), list.end(), compDesc);

    QCOMPARE(list[0].id(), 2); // newest upload date (5000) first
    QCOMPARE(list[1].id(), 1); // oldest upload date (1000) second
}

void TestChapterSort::testAlphabeticalSort()
{
    Manga manga;
    manga.setChapterFlags(Manga::CHAPTER_SORTING_ALPHABET);

    Chapter ca(1, 1, "/a", "Alpha", "", false, false, 0, 0.0f, 0, 0, 0, 0, 0, 0);
    Chapter cb(2, 1, "/b", "Beta", "", false, false, 0, 0.0f, 0, 0, 0, 0, 0, 0);
    Chapter cz(3, 1, "/z", "Zeta", "", false, false, 0, 0.0f, 0, 0, 0, 0, 0, 0);

    QList<Chapter> list = {cz, ca, cb};
    auto compAsc = getChapterComparator(manga, /*sortDescending=*/false);
    std::sort(list.begin(), list.end(), compAsc);

    QCOMPARE(list[0].name(), QString("Alpha"));
    QCOMPARE(list[1].name(), QString("Beta"));
    QCOMPARE(list[2].name(), QString("Zeta"));
}

void TestChapterSort::testStrictWeakOrdering()
{
    Manga manga;
    auto comp = getChapterComparator(manga, false);

    Chapter c1(1, 1, "/1", "Ch 1", "", false, false, 0, 1.0f, 0, 0, 0, 0, 0, 0);
    Chapter c2(2, 1, "/2", "Ch 2", "", false, false, 0, 2.0f, 1, 0, 0, 0, 0, 0);
    Chapter c3(3, 1, "/3", "Ch 3", "", false, false, 0, 3.0f, 2, 0, 0, 0, 0, 0);

    // 1. Irreflexivity: comp(x, x) must be false
    QVERIFY(!comp(c1, c1));
    QVERIFY(!comp(c2, c2));

    // 2. Asymmetry: if comp(x, y) is true, comp(y, x) must be false
    if (comp(c1, c2)) {
        QVERIFY(!comp(c2, c1));
    }

    // 3. Transitivity: if comp(x, y) and comp(y, z), then comp(x, z)
    if (comp(c3, c2) && comp(c2, c1)) {
        QVERIFY(comp(c3, c1));
    }
}

QTEST_MAIN(TestChapterSort)
#include "test_chapter_sort.moc"
