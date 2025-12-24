#ifndef MANGAREPOSITORY_H
#define MANGAREPOSITORY_H

#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDateTime>

// Forward declaration of Manga model
class Manga;

class MangaRepository
{
public:
    explicit MangaRepository(); // Removed QSqlDatabase argument

    // CRUD operations
    bool insertManga(const Manga& manga);
    bool updateManga(const Manga& manga);
    Manga getMangaById(long id);
    QList<Manga> getAllManga();
    QList<Manga> getFavorites();
    bool deleteManga(long id);
    Manga getMangaByUrl(const QString& url, long sourceId);

private:
    // QSqlDatabase m_db; // Removed member // Store by value

    Manga mangaFromQuery(QSqlQuery& query);
};

#endif // MANGAREPOSITORY_H
