#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QString>
#include <QList>
#include <QObject> // Inherit from QObject for signals/slots and QJSEngine integration

// Forward declarations for models
class Manga; // From src/model/Manga.h
class Chapter; // From src/model/Chapter.h
#include "model/SChapter.h" // Include the correct SChapter definition

class SourceBase : public QObject
{
    Q_OBJECT
public:
    explicit SourceBase(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~SourceBase() = default;

    virtual QString name() const = 0;
    virtual long id() const = 0;
    virtual QString lang() const = 0;
    virtual QString baseUrl() const = 0;
    virtual bool supportsLatest() const = 0;

    // Browse related (page is 1-based; implementations must support
    // incremental paging so the browse view can load continuously)
    virtual QList<Manga> getPopularManga(int page) = 0;
    virtual QList<Manga> getLatestUpdates(int page) = 0;
    virtual QList<Manga> getSearchManga(const QString& query, int page) = 0;

    // Manga details related
    virtual Manga getMangaDetails(const Manga& manga) = 0;

    // Chapters
    virtual QList<SChapter> getChapterList(const Manga& manga) = 0;
    virtual QList<QString> getPageList(const Chapter& chapter) = 0;

signals:
    // Can add signals here for progress, errors, etc.
};

#endif // SOURCEBASE_H
