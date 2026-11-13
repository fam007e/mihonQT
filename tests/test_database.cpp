#include <QTest>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "../src/database/DatabaseManager.h"
#include "../src/database/MangaRepository.h"
#include "../src/database/ChapterRepository.h"
#include "../src/model/Manga.h"
#include "../src/model/Chapter.h"

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMangaInsertAndNotesDefault();
    void testChapterInsertAndOrder();
};

void TestDatabase::initTestCase()
{
    // Initialize in-memory database
    bool ok = DatabaseManager::instance().openDatabase(":memory:");
    QVERIFY(ok);
}

void TestDatabase::cleanupTestCase()
{
    DatabaseManager::instance().closeDatabase();
}

void TestDatabase::testMangaInsertAndNotesDefault()
{
    MangaRepository& repo = DatabaseManager::instance().mangaRepository();

    // Create manga with empty notes (which was failing before schema/repo fix)
    Manga manga;
    manga.setTitle("Test Manga");
    manga.setUrl("/manga/test");
    manga.setSource(1234567);
    manga.setNotes(""); // empty notes

    bool inserted = repo.insertManga(manga);
    QVERIFY(inserted);

    Manga fetched = repo.getMangaByUrl("/manga/test", 1234567);
    QVERIFY(fetched.id() > 0);
    QCOMPARE(fetched.title(), QString("Test Manga"));
    QCOMPARE(fetched.notes(), QString(""));
}

void TestDatabase::testChapterInsertAndOrder()
{
    MangaRepository& mangaRepo = DatabaseManager::instance().mangaRepository();
    ChapterRepository& chapterRepo = DatabaseManager::instance().chapterRepository();

    Manga manga = mangaRepo.getMangaByUrl("/manga/test", 1234567);
    QVERIFY(manga.id() > 0);

    // Insert chapters in a transaction
    QSqlDatabase::database().transaction();
    for (int i = 0; i < 5; ++i) {
        Chapter chap(
            -1,
            manga.id(),
            QString("/manga/test/ch%1").arg(i + 1),
            QString("Chapter %1").arg(i + 1),
            "ScanGroup",
            false,
            false,
            0,
            static_cast<float>(i + 1),
            4 - i, // sourceOrder (0 = newest)
            0,
            1000 + i,
            0,
            0,
            0
        );
        QVERIFY(chapterRepo.insertChapter(chap));
    }
    QSqlDatabase::database().commit();

    QList<Chapter> chapters = chapterRepo.getChaptersByMangaId(manga.id());
    QCOMPARE(chapters.size(), 5);
}

QTEST_MAIN(TestDatabase)
#include "test_database.moc"
