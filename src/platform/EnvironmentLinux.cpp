#include "Environment.h"
#include "Logger.h"
#include <cstdlib>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

class EnvironmentLinux : public Environment
{
public:
    QString get(const char *key) override
    {
        const char *value = getenv(key);
        return value ? QString(value) : QString();
    }

    void set(const char *key, const char *value) override
    {
        QString confDir = QDir(QStandardPaths::writableLocation(
            QStandardPaths::GenericConfigLocation)).filePath("environment.d");
        if (!QDir(confDir).exists()) {
            QDir().mkpath(confDir);
        }

        QString confPath = QDir(confDir).filePath("gamepadtool.conf");
        QFile f(confPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            Logger::instance().error(QString("Can't write %1").arg(confPath));
            return;
        }
        QTextStream stream(&f);
        stream << key << "=" << value << "\n";
        stream.flush();
        f.close();
    }

    void unset(const char *key) override
    {
        Q_UNUSED(key);
        QString confPath = QDir(QStandardPaths::writableLocation(
            QStandardPaths::GenericConfigLocation)).filePath("environment.d/gamepadtool.conf");
        QFile::remove(confPath);
    }
};

std::unique_ptr<Environment> Environment::create()
{
    return std::make_unique<EnvironmentLinux>();
}
