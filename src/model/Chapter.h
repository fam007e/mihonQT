#ifndef CHAPTER_H
#define CHAPTER_H

#include <QString>

class Chapter
{
public:
    Chapter(long id = -1,
            long mangaId = -1,
            const QString& url = "",
            const QString& name = "",
            const QString& scanlator = "",
            bool read = false,
            bool bookmark = false,
            long lastPageRead = 0,
            double chapterNumber = -1.0,
            long sourceOrder = 0,
            long dateFetch = 0,
            long dateUpload = 0,
            long lastModifiedAt = 0,
            long version = 0,
            bool isSyncing = false);

    // Getters
    long id() const { return m_id; }
    long mangaId() const { return m_mangaId; }
    const QString& url() const { return m_url; }
    const QString& name() const { return m_name; }
    const QString& scanlator() const { return m_scanlator; }
    bool read() const { return m_read; }
    bool bookmark() const { return m_bookmark; }
    long lastPageRead() const { return m_lastPageRead; }
    double chapterNumber() const { return m_chapterNumber; }
    long sourceOrder() const { return m_sourceOrder; }
    long dateFetch() const { return m_dateFetch; }
    long dateUpload() const { return m_dateUpload; }
    long lastModifiedAt() const { return m_lastModifiedAt; }
    long version() const { return m_version; }
    bool isSyncing() const { return m_isSyncing; }

    // Setters
    void setId(long id) { m_id = id; }
    void setMangaId(long mangaId) { m_mangaId = mangaId; }
    void setUrl(const QString& url) { m_url = url; }
    void setName(const QString& name) { m_name = name; }
    void setScanlator(const QString& scanlator) { m_scanlator = scanlator; }
    void setRead(bool read) { m_read = read; }
    void setBookmark(bool bookmark) { m_bookmark = bookmark; }
    void setLastPageRead(long lastPageRead) { m_lastPageRead = lastPageRead; }
    void setChapterNumber(double chapterNumber) { m_chapterNumber = chapterNumber; }
    void setSourceOrder(long sourceOrder) { m_sourceOrder = sourceOrder; }
    void setDateFetch(long dateFetch) { m_dateFetch = dateFetch; }
    void setDateUpload(long dateUpload) { m_dateUpload = dateUpload; }
    void setLastModifiedAt(long lastModifiedAt) { m_lastModifiedAt = lastModifiedAt; }
    void setVersion(long version) { m_version = version; }
    void setIsSyncing(bool isSyncing) { m_isSyncing = isSyncing; }

private:
    long m_id;
    long m_mangaId;
    QString m_url;
    QString m_name;
    QString m_scanlator;
    bool m_read;
    bool m_bookmark;
    long m_lastPageRead;
    double m_chapterNumber;
    long m_sourceOrder;
    long m_dateFetch;
    long m_dateUpload;
    long m_lastModifiedAt;
    long m_version;
    bool m_isSyncing;
};

#endif // CHAPTER_H
