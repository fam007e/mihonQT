#include "SChapter.h"

SChapter::SChapter(const QString& url, const QString& name, long dateUpload, float chapterNumber, const QString& scanlator)
    : m_url(url),
      m_name(name),
      m_dateUpload(dateUpload),
      m_chapterNumber(chapterNumber),
      m_scanlator(scanlator)
{
}