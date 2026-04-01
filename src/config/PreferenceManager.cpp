#include "PreferenceManager.h"

const QString PreferenceManager::ENFORCE_HTTPS = "enforce_https";
const QString PreferenceManager::TRUSTED_EXTENSIONS = "trusted_extensions";

PreferenceManager& PreferenceManager::instance()
{
    static PreferenceManager instance;
    return instance;
}

PreferenceManager::PreferenceManager()
    : m_settings("MihonQT", "MihonQT")
{
}

PreferenceManager::~PreferenceManager()
{
}

void PreferenceManager::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
}

QVariant PreferenceManager::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}
