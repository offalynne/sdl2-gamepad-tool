#include "Environment.h"
#include "Logger.h"
#include <QSettings>

class EnvironmentWin : public Environment
{
public:
    QString get(const char *key) override
    {
        QSettings settings("HKEY_CURRENT_USER\\Environment", QSettings::NativeFormat);
        QString res = settings.value(key, "").toString();
        return res;
    }

    void set(const char *key, const char *value) override
    {
        QSettings settings("HKEY_CURRENT_USER\\Environment", QSettings::NativeFormat);
        settings.setValue(key, value);
        settings.sync();
        if (settings.status() != QSettings::NoError) {
            Logger::instance().error(QString("Failed to set registry key \"%1\"").arg(key));
        }
    }

    void unset(const char *key) override
    {
        QSettings settings("HKEY_CURRENT_USER\\Environment", QSettings::NativeFormat);
        settings.remove(key);
        settings.sync();
        if (settings.status() != QSettings::NoError) {
            Logger::instance().error(QString("Failed to remove registry key \"%1\"").arg(key));
        }
    }
};

std::unique_ptr<Environment> Environment::create()
{
    return std::make_unique<EnvironmentWin>();
}
