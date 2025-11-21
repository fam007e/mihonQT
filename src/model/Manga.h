#ifndef MANGA_H
#define MANGA_H

#include <QString>
#include <QList>
#include <QDateTime>

// Enum for Manga Status
// Matches SManga.kt status constants (UNKNOWN, ONGOING, COMPLETED, LICENSED, PUBLISHING_FINISHED, CANCELLED, ON_HIATUS)
enum MangaStatus {
    UNKNOWN = 0,
    ONGOING = 1,
    COMPLETED = 2,
    LICENSED = 3,
    PUBLISHING_FINISHED = 4,
    CANCELLED = 5,
    ON_HIATUS = 6
};

// Enum for UpdateStrategy (matches UpdateStrategy.kt from source-api)
enum UpdateStrategy {
    ALWAYS_UPDATE = 0,
    ONLY_FETCH_ONCE = 1
};

class Manga
{
public:
    Manga(long id = -1,
          long source = -1,
          const QString& url = "",
          const QString& title = "",
          const QString& artist = "",
          const QString& author = "",
          const QString& description = "",
          const QString& genre = "", // Stored as comma-separated string
          int status = MangaStatus::UNKNOWN,
          const QString& thumbnailUrl = "",
          bool favorite = false,
          long lastUpdate = 0,
          long nextUpdate = 0,
          int fetchInterval = 0,
          long dateAdded = 0,
          long viewerFlags = 0,
          long chapterFlags = 0,
          long coverLastModified = 0,
          UpdateStrategy updateStrategy = UpdateStrategy::ALWAYS_UPDATE,
          bool initialized = false,
          long lastModifiedAt = 0,
          long favoriteModifiedAt = 0,
          long version = 0,
          const QString& notes = "",
          bool isSyncing = false);

    // Getters
    long id() const { return m_id; }
    long source() const { return m_source; }
    const QString& url() const { return m_url; }
    const QString& title() const { return m_title; }
    const QString& artist() const { return m_artist; }
    const QString& author() const { return m_author; }
    const QString& description() const { return m_description; }
    const QString& genre() const { return m_genre; }
    int status() const { return m_status; }
    const QString& thumbnailUrl() const { return m_thumbnailUrl; }
    bool favorite() const { return m_favorite; }
    long lastUpdate() const { return m_lastUpdate; }
    long nextUpdate() const { return m_nextUpdate; }
    int fetchInterval() const { return m_fetchInterval; }
    long dateAdded() const { return m_dateAdded; }
    long viewerFlags() const { return m_viewerFlags; }
    long chapterFlags() const { return m_chapterFlags; }
    long coverLastModified() const { return m_coverLastModified; }
    UpdateStrategy updateStrategy() const { return m_updateStrategy; }
    bool initialized() const { return m_initialized; }
    long lastModifiedAt() const { return m_lastModifiedAt; }
    long favoriteModifiedAt() const { return m_favoriteModifiedAt; }
    long version() const { return m_version; }
    const QString& notes() const { return m_notes; }
    bool isSyncing() const { return m_isSyncing; }

    // Setters
    void setId(long id) { m_id = id; }
    void setSource(long source) { m_source = source; }
    void setUrl(const QString& url) { m_url = url; }
    void setTitle(const QString& title) { m_title = title; }
    void setArtist(const QString& artist) { m_artist = artist; }
    void setAuthor(const QString& author) { m_author = author; }
    void setDescription(const QString& description) { m_description = description; }
    void setGenre(const QString& genre) { m_genre = genre; }
    void setStatus(int status) { m_status = status; }
    void setThumbnailUrl(const QString& thumbnailUrl) { m_thumbnailUrl = thumbnailUrl; }
    void setFavorite(bool favorite) { m_favorite = favorite; }
    void setLastUpdate(long lastUpdate) { m_lastUpdate = lastUpdate; }
    void setNextUpdate(long nextUpdate) { m_nextUpdate = nextUpdate; }
    void setFetchInterval(int fetchInterval) { m_fetchInterval = fetchInterval; }
    void setDateAdded(long dateAdded) { m_dateAdded = dateAdded; }
    void setViewerFlags(long viewerFlags) { m_viewerFlags = viewerFlags; }
    void setChapterFlags(long chapterFlags) { m_chapterFlags = chapterFlags; }
    void setCoverLastModified(long coverLastModified) { m_coverLastModified = coverLastModified; }
    void setUpdateStrategy(UpdateStrategy updateStrategy) { m_updateStrategy = updateStrategy; }
    void setInitialized(bool initialized) { m_initialized = initialized; }
    void setLastModifiedAt(long lastModifiedAt) { m_lastModifiedAt = lastModifiedAt; }
    void setFavoriteModifiedAt(long favoriteModifiedAt) { m_favoriteModifiedAt = favoriteModifiedAt; }
    void setVersion(long version) { m_version = version; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setIsSyncing(bool isSyncing) { m_isSyncing = isSyncing; }

private:
    long m_id;
    long m_source;
    QString m_url;
    QString m_title;
    QString m_artist;
    QString m_author;
    QString m_description;
    QString m_genre;
    int m_status;
    QString m_thumbnailUrl;
    bool m_favorite;
    long m_lastUpdate;
    long m_nextUpdate;
    int m_fetchInterval;
    long m_dateAdded;
    long m_viewerFlags;
    long m_chapterFlags;
    long m_coverLastModified;
    UpdateStrategy m_updateStrategy;
    bool m_initialized;
    long m_lastModifiedAt;
    long m_favoriteModifiedAt;
    long m_version;
    QString m_notes;
    bool m_isSyncing;
};

#endif // MANGA_H
