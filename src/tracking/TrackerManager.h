#ifndef TRACKERMANAGER_H
#define TRACKERMANAGER_H

#include <QObject>
#include <QList>
#include "TrackingService.h"

class TrackerManager : public QObject
{
    Q_OBJECT

public:
    static TrackerManager& instance();

    QList<TrackingService*> trackers() const;
    TrackingService* tracker(const QString& id) const;

    // Updates all bound trackers for this manga if tracking/auto_update is enabled
    void updateProgress(long localMangaId, int chaptersRead);

signals:
    void progressUpdated(long localMangaId, int chaptersRead);

private:
    explicit TrackerManager(QObject *parent = nullptr);
};

#endif // TRACKERMANAGER_H
