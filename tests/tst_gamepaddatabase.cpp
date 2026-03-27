#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QSignalSpy>
#include "GamepadDatabase.h"
#include "Logger.h"

class TestGamepadDatabase : public QObject
{
    Q_OBJECT

private slots:
    void testConstants();
    void testInitializeCreatesDataDir();
    void testSaveLocalMappingCreatesFile();
    void testSaveLocalMappingReplacesExistingGuid();
    void testSaveLocalMappingAppendsNewGuid();
    void testSaveLocalMappingSameGuidDifferentCrcKeptSeparate();
    void testSaveLocalMappingSameGuidSameCrcReplaces();
    void testSaveLocalMappingNameContainingCrcTag();
    void testProcessDownloadedDataEmptyReturns();
    void testProcessDownloadedDataSameHash();
    void testProcessDownloadedDataDifferentHash();
};

void TestGamepadDatabase::testConstants()
{
    QCOMPARE(QString(GamepadDatabase::DB_FILE), QString("gamecontrollerdb.txt"));
    QCOMPARE(QString(GamepadDatabase::LOCAL_DB_FILE), QString("gamecontrollerdb.local.txt"));
    QVERIFY(QString(GamepadDatabase::DB_URL).contains("gamecontrollerdb.txt"));
}

void TestGamepadDatabase::testInitializeCreatesDataDir()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    GamepadDatabase db;
    db.setDataPath(tmpDir.path() + "/testdata");

    // Verify the path was set
    QCOMPARE(db.dataPath(), tmpDir.path() + "/testdata");
}

void TestGamepadDatabase::testSaveLocalMappingCreatesFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());

    QString mapping = "030000004c050000c405000000010000,PS4 Controller,a:b1,b:b2,";
    db.saveLocalMapping(mapping);

    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.exists());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();
    QVERIFY(content.contains("PS4 Controller"));
    QVERIFY(content.contains("030000004c050000c405000000010000"));
}

void TestGamepadDatabase::testSaveLocalMappingReplacesExistingGuid()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create a local DB with an existing entry
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("aabb00001122334455660000,Old Gamepad,a:b0,b:b1,\n");
    f.write("other00001122334455660000,Other Gamepad,a:b0,\n");
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());
    QString newMapping = "aabb00001122334455660000,New Gamepad,a:b2,b:b3,x:b4,";
    db.saveLocalMapping(newMapping);

    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();

    // Old entry should be gone, new entry present
    QVERIFY(!content.contains("Old Gamepad"));
    QVERIFY(content.contains("New Gamepad"));
    // Other entry should be preserved
    QVERIFY(content.contains("Other Gamepad"));
}

void TestGamepadDatabase::testSaveLocalMappingAppendsNewGuid()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("existing00001122334455660000,Existing Gamepad,a:b0,\n");
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());
    QString newMapping = "newguid00001122334455660000,New Gamepad,a:b1,";
    db.saveLocalMapping(newMapping);

    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();

    QVERIFY(content.contains("Existing Gamepad"));
    QVERIFY(content.contains("New Gamepad"));
}

void TestGamepadDatabase::testSaveLocalMappingSameGuidDifferentCrcKeptSeparate()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Pre-populate with a mapping that has crc:1111
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("030000004c050000c405000000010000,PS4 CRC-1111,a:b0,b:b1,crc:1111,\n");
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());

    // Save a mapping with the same GUID but different CRC
    QString newMapping = "030000004c050000c405000000010000,PS4 CRC-2222,a:b2,b:b3,crc:2222,";
    db.saveLocalMapping(newMapping);

    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();

    // Both entries should be present — different CRC means different controller variant
    QVERIFY(content.contains("CRC-1111"));
    QVERIFY(content.contains("CRC-2222"));
}

void TestGamepadDatabase::testSaveLocalMappingSameGuidSameCrcReplaces()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Pre-populate with a mapping that has crc:aaaa
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("030000004c050000c405000000010000,PS4 Old,a:b0,b:b1,crc:aaaa,\n");
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());

    // Save a mapping with the same GUID and same CRC — should replace
    QString newMapping = "030000004c050000c405000000010000,PS4 New,a:b2,b:b3,crc:aaaa,";
    db.saveLocalMapping(newMapping);

    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();

    // Old entry should be replaced
    QVERIFY(!content.contains("PS4 Old"));
    QVERIFY(content.contains("PS4 New"));
}

void TestGamepadDatabase::testSaveLocalMappingNameContainingCrcTag()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Pre-populate with a mapping whose controller name contains "crc:"
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::LOCAL_DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("030000004c050000c405000000010000,My crc:pad,a:b0,b:b1,crc:aaaa,\n");
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());

    // Save again with same GUID and same real CRC — should replace, not duplicate
    QString newMapping = "030000004c050000c405000000010000,My crc:pad v2,a:b2,b:b3,crc:aaaa,";
    db.saveLocalMapping(newMapping);

    QVERIFY(f.open(QIODevice::ReadOnly));
    QString content = f.readAll();
    f.close();

    // Old entry must be replaced, not duplicated
    QVERIFY(!content.contains("My crc:pad,"));
    QVERIFY(content.contains("My crc:pad v2"));
    // Only one mapping line should exist (plus trailing newline)
    QCOMPARE(content.count("030000004c050000c405000000010000"), 1);
}

void TestGamepadDatabase::testProcessDownloadedDataEmptyReturns()
{
    GamepadDatabase db;
    QVERIFY(!db.processDownloadedData(QByteArray()));
}

void TestGamepadDatabase::testProcessDownloadedDataSameHash()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    QByteArray dbContent = "# game controller db\nsome,mapping,data\n";
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(dbContent);
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);

    // Same data = same hash = no update
    bool result = db.processDownloadedData(dbContent);
    QVERIFY(!result);
    QVERIFY(spy.count() >= 1);
    QVERIFY(spy.last().at(0).toString().contains("up-to-date"));
}

void TestGamepadDatabase::testProcessDownloadedDataDifferentHash()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    QByteArray oldContent = "# old db\nold,mapping,\n";
    QFile f(QDir(tmpDir.path()).filePath(GamepadDatabase::DB_FILE));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(oldContent);
    f.close();

    GamepadDatabase db;
    db.setDataPath(tmpDir.path());
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);

    QByteArray newContent = "# new db\nnew,mapping,data\n";
    bool result = db.processDownloadedData(newContent);
    QVERIFY(result);
    QVERIFY(spy.count() >= 1);
    QVERIFY(spy.last().at(0).toString().contains("New mappings"));

    // Verify file was actually updated
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.readAll(), newContent);
    f.close();
}

QTEST_MAIN(TestGamepadDatabase)
#include "tst_gamepaddatabase.moc"
