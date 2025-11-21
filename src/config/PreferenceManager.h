#ifndef PREFERENCEMANAGER_H
#define PREFERENCEMANAGER_H

#include <QSettings>
#include <QString>
#include <QVariant>

class PreferenceManager
{
public:
    static PreferenceManager& instance();

    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
    PreferenceManager();
    ~PreferenceManager();
    PreferenceManager(const PreferenceManager&) = delete;
    PreferenceManager& operator=(const PreferenceManager&) = delete;

    QSettings m_settings;
};

#endif // PREFERENCEMANAGER_H
