#ifndef GAMEPAD_DATABASE_H
#define GAMEPAD_DATABASE_H

#include <QByteArray>
#include <QString>

class GamepadDatabase
{
public:
    static constexpr const char* DB_FILE = "gamecontrollerdb.txt";
    static constexpr const char* LOCAL_DB_FILE = "gamecontrollerdb.local.txt";
    static constexpr const char* DB_URL =
        "https://raw.githubusercontent.com/mdqinc/SDL_GameControllerDB/master/gamecontrollerdb.txt";

    // Set up the data directory and copy bundled DB if needed.
    // appDirPath is QApplication::applicationDirPath().
    void initialize(const QString& appDirPath);

    // Load SDL controller mappings from both DB files.
    void loadMappings();

    // Save a single mapping string to the local DB file.
    // Replaces any existing line whose GUID+CRC key matches.
    void saveLocalMapping(const QString& mappingString);

    // Check if a local mapping exists for the given GUID string.
    bool hasLocalMapping(const QString& guid) const;

    // Delete a local mapping for the given GUID string.
    // Returns true if a mapping was found and removed.
    bool deleteLocalMapping(const QString& guid);

    // Process downloaded DB data: compare hash with current DB file.
    // If different, writes the new data to the DB file.
    // Returns true if the DB was updated (new mappings available).
    bool processDownloadedData(const QByteArray& data);

    // Returns the data directory path (set during initialize).
    QString dataPath() const { return m_dataPath; }

    // Set data path directly (for testing).
    void setDataPath(const QString& path) { m_dataPath = path; }

private:
    QString m_dataPath;
};

#endif // GAMEPAD_DATABASE_H
