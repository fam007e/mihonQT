#include "SourceManager.h"
#include "SourceBase.h"
#include "SourceManager.h"
#include "SourceBase.h"
#include "JavascriptSource.h"
#include <QDebug> // For qWarning
#include <QDir>
#include <QDirIterator>

SourceManager::SourceManager(QObject *parent)
    : QObject(parent)
{
}

SourceManager::~SourceManager()
{
    // SourceManager does not own the SourceBase pointers,
    // they should be managed by their respective owners (e.g., MainWindow)
    // or by a QObject parent/ownership system.
    // So no deletion here.
}

void SourceManager::addSource(SourceBase* source)
{
    if (!source) {
        return;
    }

    // Check for duplicates before adding
    // Note: IDs and names should ideally be unique. This provides a basic check.
    if (m_sourcesById.contains(source->id())) {
        qWarning() << "Source with ID" << source->id() << "(" << source->name() << ")" << "already exists. Skipping.";
        return;
    }
    if (m_sourcesByName.contains(source->name())) {
        qWarning() << "Source with name" << source->name() << "(" << source->id() << ")" << "already exists. Skipping.";
        return;
    }


    m_sources.append(source);
    m_sourcesById.insert(source->id(), source);
    m_sourcesByName.insert(source->name(), source);
    m_sourcesByName.insert(source->name(), source);
}

void SourceManager::loadExtensions(const QString& directoryPath, QJSEngine* engine, NetworkAccessManager* networkManager)
{
    qDebug() << "Loading extensions from:" << directoryPath;
    QDir dir(directoryPath);
    if (!dir.exists()) {
        qWarning() << "Extensions directory does not exist:" << directoryPath;
        // Try to create it? Or just return.
        // dir.mkpath(".");
        return;
    }

    QDirIterator it(directoryPath, QStringList() << "*.js", QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString scriptPath = it.next();
        qDebug() << "Found extension script:" << scriptPath;
        JavascriptSource* source = new JavascriptSource(scriptPath, engine, networkManager, this);
        // JavascriptSource init might fail if script is bad, but we add it anyway so we can see it in UI (maybe with error state?)
        // Ideally JavascriptSource should have an isValid() check.
        // For now, assuming it parses okay or logs errors.

        // Check if ID/Name valid?
        if (source->id() <= 0) {
            qWarning() << "Skipping invalid source (invalid ID) from:" << scriptPath;
            delete source;
            continue;
        }

        addSource(source);
    }
}


SourceBase* SourceManager::getSourceById(long id) const
{
    return m_sourcesById.value(id, nullptr);
}

SourceBase* SourceManager::getSourceByName(const QString& name) const
{
    return m_sourcesByName.value(name, nullptr);
}

QList<SourceBase*> SourceManager::getAllSources() const
{
    return m_sources;
}
