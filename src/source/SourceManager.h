#ifndef SOURCEMANAGER_H
#define SOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>

class SourceBase; // Forward declaration

class SourceManager : public QObject
{
    Q_OBJECT
public:
    explicit SourceManager(QObject *parent = nullptr);
    ~SourceManager() override;

    void addSource(SourceBase* source);
    SourceBase* getSourceById(long id) const;
    SourceBase* getSourceByName(const QString& name) const;
    QList<SourceBase*> getAllSources() const;

private:
    QList<SourceBase*> m_sources;
    QMap<long, SourceBase*> m_sourcesById;
    QMap<QString, SourceBase*> m_sourcesByName;
};

#endif // SOURCEMANAGER_H