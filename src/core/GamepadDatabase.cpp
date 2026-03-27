#include "GamepadDatabase.h"
#include "Logger.h"
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QCryptographicHash>
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_gamecontroller.h"

void GamepadDatabase::initialize(const QString& appDirPath)
{
    QString fullDbPath = QDir(appDirPath).filePath(DB_FILE);

#ifdef LINUX_BUILD
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif

    if (!QDir(m_dataPath).exists()) {
        QDir().mkpath(m_dataPath);
    }
    Logger::instance().info(m_dataPath);

    QString destDbPath = QDir(m_dataPath).filePath(DB_FILE);
    if (!QFile::exists(destDbPath)) {
        if (!QFile::copy(fullDbPath, destDbPath)) {
            Logger::instance().error(QString("Failed to copy database to %1").arg(destDbPath));
        }
    }
}

void GamepadDatabase::loadMappings()
{
    QString dbPath = QDir(m_dataPath).filePath(DB_FILE);
    QString localDbPath = QDir(m_dataPath).filePath(LOCAL_DB_FILE);
    SDL_GameControllerAddMappingsFromFile(dbPath.toUtf8().constData());
    SDL_GameControllerAddMappingsFromFile(localDbPath.toUtf8().constData());
}

static QString extractMappingKey(const QString& mapping)
{
    // Build a key from GUID + CRC so controllers sharing a base GUID but
    // differing by CRC are stored as separate entries.
    QString key;
    int commaPos = mapping.indexOf(',');
    if (commaPos > 0) {
        key = mapping.left(commaPos);
    }
    static const QString crcTag = QStringLiteral(",crc:");
    int crcPos = mapping.indexOf(crcTag);
    if (crcPos >= 0) {
        crcPos += 1; // skip the leading comma to capture "crc:XXXX"
        int crcEnd = mapping.indexOf(',', crcPos);
        if (crcEnd < 0)
            crcEnd = mapping.length();
        key += ',' + mapping.mid(crcPos, crcEnd - crcPos);
    }
    return key;
}

void GamepadDatabase::saveLocalMapping(const QString& mappingString)
{
    QString key = extractMappingKey(mappingString);
    QString filePath = QDir(m_dataPath).filePath(LOCAL_DB_FILE);

    // Read phase: collect existing lines, filtering out the old entry for this key
    QString contents;
    QFile readFile(filePath);
    if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream t(&readFile);
        while (!t.atEnd()) {
            QString line = t.readLine();
            if (!key.isEmpty() && extractMappingKey(line) == key) {
                continue; // skip existing entry for this GUID+CRC
            }
            contents.append(line + "\n");
        }
        readFile.close();
    }
    contents.append(mappingString);
    contents.append("\n");

    // Write phase: atomic write via temp file + rename
    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::instance().error(QString("Failed to open %1 for writing").arg(filePath));
        return;
    }
    QTextStream out(&saveFile);
    out << contents;
    if (!saveFile.commit()) {
        Logger::instance().error(QString("Failed to save local mappings to %1").arg(filePath));
    }
}

static QString stripCrcFromGuid(const QString& guid)
{
    // SDL2 embeds the CRC in GUID bytes 2-3 (hex chars 4-7).
    // Replace them with zeros to get the base GUID for matching.
    if (guid.length() >= 32) {
        return guid.left(4) + "0000" + guid.mid(8);
    }
    return guid;
}

bool GamepadDatabase::hasLocalMapping(const QString& guid) const
{
    QString baseGuid = stripCrcFromGuid(guid);
    QString filePath = QDir(m_dataPath).filePath(LOCAL_DB_FILE);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream t(&file);
    while (!t.atEnd()) {
        QString line = t.readLine().trimmed();
        int commaPos = line.indexOf(',');
        if (commaPos > 0) {
            QString lineGuid = stripCrcFromGuid(line.left(commaPos));
            if (lineGuid == baseGuid) {
                return true;
            }
        }
    }
    return false;
}

bool GamepadDatabase::deleteLocalMapping(const QString& guid)
{
    QString baseGuid = stripCrcFromGuid(guid);
    QString filePath = QDir(m_dataPath).filePath(LOCAL_DB_FILE);
    QFile readFile(filePath);
    if (!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString contents;
    bool found = false;
    QTextStream t(&readFile);
    while (!t.atEnd()) {
        QString line = t.readLine();
        QString trimmed = line.trimmed();
        int commaPos = trimmed.indexOf(',');
        if (commaPos > 0 && stripCrcFromGuid(trimmed.left(commaPos)) == baseGuid) {
            found = true;
            continue;
        }
        contents.append(line + "\n");
    }
    readFile.close();

    if (!found) {
        return false;
    }

    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::instance().error(QString("Failed to open %1 for writing").arg(filePath));
        return false;
    }
    QTextStream out(&saveFile);
    out << contents;
    if (!saveFile.commit()) {
        Logger::instance().error(QString("Failed to save local mappings to %1").arg(filePath));
        return false;
    }
    return true;
}

bool GamepadDatabase::processDownloadedData(const QByteArray& data)
{
    if (data.isEmpty()) {
        return false;
    }

    QCryptographicHash newHash(QCryptographicHash::Sha1);
    newHash.addData(data);

    QFile dbFile(QDir(m_dataPath).filePath(DB_FILE));
    if (!dbFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QCryptographicHash currentHash(QCryptographicHash::Sha1);
    currentHash.addData(dbFile.readAll());
    dbFile.close();

    if (newHash.result() == currentHash.result()) {
        Logger::instance().info("Mappings are already up-to-date");
        return false;
    }

    // Hashes differ — atomic write via temp file + rename
    QSaveFile saveFile(QDir(m_dataPath).filePath(DB_FILE));
    if (!saveFile.open(QIODevice::WriteOnly)) {
        Logger::instance().error("Failed to write updated mappings");
        return false;
    }
    saveFile.write(data);
    if (!saveFile.commit()) {
        Logger::instance().error("Failed to save updated mappings");
        return false;
    }
    Logger::instance().info("New mappings available");
    return true;
}
