#ifndef SCHAPTER_H
#define SCHAPTER_H

#include <QString>

class SChapter
{
public:
    SChapter(const QString& url = "",
             const QString& name = "",
             long dateUpload = 0,
             float chapterNumber = 0.0f,
             const QString& scanlator = "");

    // Getters
    const QString& url() const { return m_url; }
    const QString& name() const { return m_name; }
    long dateUpload() const { return m_dateUpload; }
    float chapterNumber() const { return m_chapterNumber; }
    const QString& scanlator() const { return m_scanlator; }

    // Setters
    void setUrl(const QString& url) { m_url = url; }
    void setName(const QString& name) { m_name = name; }
    void setDateUpload(long dateUpload) { m_dateUpload = dateUpload; }
    void setChapterNumber(float chapterNumber) { m_chapterNumber = chapterNumber; }
    void setScanlator(const QString& scanlator) { m_scanlator = scanlator; }

private:
    QString m_url;
    QString m_name;
    long m_dateUpload;
    float m_chapterNumber;
    QString m_scanlator;
};

#endif // SCHAPTER_H
