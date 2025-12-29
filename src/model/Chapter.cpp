#include "Chapter.h"

Chapter::Chapter(long id, long mangaId, const QString& url, const QString& name,
                 const QString& scanlator, bool read, bool bookmark, long lastPageRead, // flawfinder: ignore
                 double chapterNumber, long sourceOrder, long dateFetch, long dateUpload,
                 long lastModifiedAt, long version, bool isSyncing)
    : m_id(id),
      m_mangaId(mangaId),
      m_url(url),
      m_name(name),
      m_scanlator(scanlator),
      m_read(read), // flawfinder: ignore
      m_bookmark(bookmark),
      m_lastPageRead(lastPageRead),
      m_chapterNumber(chapterNumber),
      m_sourceOrder(sourceOrder),
      m_dateFetch(dateFetch),
      m_dateUpload(dateUpload),
      m_lastModifiedAt(lastModifiedAt),
      m_version(version),
      m_isSyncing(isSyncing)
{
}
