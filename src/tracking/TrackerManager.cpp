#include "TrackerManager.h"
#include "AniListTracker.h"
#include "MyAnimeListTracker.h"
#include "MangaUpdatesTracker.h"
#include "config/PreferenceManager.h"
#include <QDebug>

TrackerManager& TrackerManager::instance()
{
    static TrackerManager instance;
    return instance;
}

TrackerManager::TrackerManager(QObject *parent)
    : QObject(parent)
{
}

QList<TrackingService*> TrackerManager::trackers() const
{
    return {
        &AniListTracker::instance(),
        &MyAnimeListTracker::instance(),
        &MangaUpdatesTracker::instance()
    };
}

TrackingService* TrackerManager::tracker(const QString& id) const
{
    for (TrackingService *t : trackers()) {
        if (t->id().compare(id, Qt::CaseInsensitive) == 0) {
            return t;
        }
    }
    return nullptr;
}

void TrackerManager::updateProgress(long localMangaId, int chaptersRead)
{
    bool autoUpdate = PreferenceManager::instance().value("tracking/auto_update", true).toBool();
    if (!autoUpdate) return;

    for (TrackingService *t : trackers()) {
        if (t->isLoggedIn()) {
            TrackingEntry entry = t->getEntry(localMangaId);
            if (entry.id > 0 && chaptersRead > entry.chaptersRead) {
                t->update(localMangaId, chaptersRead, entry.status > 0 ? entry.status : TrackingService::Reading, entry.score);
            }
        }
    }

    emit progressUpdated(localMangaId, chaptersRead);
}
