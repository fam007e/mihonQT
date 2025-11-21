#include "PreferenceManager.h"
#include <QCoreApplication>
#include <QDebug>

PreferenceManager& PreferenceManager::instance()
{
    static PreferenceManager instance;
    return instance;
}

PreferenceManager::PreferenceManager()
    : m_settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    qDebug() << "PreferenceManager initialized. Settings file:" << m_settings.fileName();
}

PreferenceManager::~PreferenceManager()
{
    qDebug() << "PreferenceManager destroyed.";
}

void PreferenceManager::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
    qDebug() << "Preference set:" << key << "=" << value;
}

QVariant PreferenceManager::value(const QString& key, const QVariant& defaultValue) const
{
    QVariant val = m_settings.value(key, defaultValue);
    qDebug() << "Preference get:" << key << "=" << val << "(default:" << defaultValue << ")";
    return val;
}
