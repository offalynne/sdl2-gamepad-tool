#include "Environment.h"
#include "Logger.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QApplication>
#include <QTextStream>

class EnvironmentMac : public Environment
{
public:
    QString get(const char *key) override
    {
        QProcess proc;
        proc.start("/bin/launchctl", QStringList() << "getenv" << key);
        proc.waitForFinished();
        return proc.readAllStandardOutput().trimmed();
    }

    void set(const char *key, const char *value) override
    {
        QProcess proc;
        proc.start("/bin/launchctl", QStringList() << "setenv" << key << value);
        proc.waitForFinished();
        if (proc.exitCode() != 0) {
            Logger::instance().error(QString("launchctl setenv %1: %2").arg(key, proc.readAllStandardError().trimmed()));
        }

        QString launchdPlistName = "local.generalarcade.gamepadtool.plist";
        QString launchdPlistLocationPlist = QDir::homePath() + "/Library/LaunchAgents/";
        QString launchdPlistPath = launchdPlistLocationPlist + launchdPlistName;
        QFile launchdPlist(launchdPlistPath);
        if (!launchdPlist.exists()) {
            if (!QDir(launchdPlistLocationPlist).exists()) {
                QDir().mkdir(launchdPlistLocationPlist);
            }
            if (!QFile::copy(QDir(QApplication::applicationDirPath()).filePath(launchdPlistName), launchdPlistPath)) {
                Logger::instance().error(QString("Failed to install LaunchAgent plist to %1").arg(launchdPlistPath));
            }
        }
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QFile f(QDir(dataPath).filePath("setenv.sh"));
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&f);
            QString escapedValue = QString(value).replace("'", "'\\''");
            stream << "launchctl setenv " << key << " '" << escapedValue << "'\n";
            stream.flush();
            f.close();
        }
    }

    void unset(const char *key) override
    {
        QProcess proc;
        proc.start("/bin/launchctl", QStringList() << "unsetenv" << key);
        proc.waitForFinished();
        if (proc.exitCode() != 0) {
            Logger::instance().error(QString("launchctl unsetenv %1: %2").arg(key, proc.readAllStandardError().trimmed()));
        }

        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QFile(QDir(dataPath).filePath("setenv.sh")).remove();
    }
};

std::unique_ptr<Environment> Environment::create()
{
    return std::make_unique<EnvironmentMac>();
}
