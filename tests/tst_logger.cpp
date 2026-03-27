#include <QTest>
#include <QSignalSpy>
#include "Logger.h"

class TestLogger : public QObject
{
    Q_OBJECT

private slots:
    void testSingleton();
    void testInfoEmitsSignal();
    void testDebugDoesNotEmitSignal();
    void testErrorEmitsSignal();
    void testInfoMessageContent();
    void testErrorMessageContent();
    void testMultipleInfoCalls();
};

void TestLogger::testSingleton()
{
    Logger& a = Logger::instance();
    Logger& b = Logger::instance();
    QCOMPARE(&a, &b);
}

void TestLogger::testInfoEmitsSignal()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().info("test info message");
    QCOMPARE(spy.count(), 1);
}

void TestLogger::testDebugDoesNotEmitSignal()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().debug("test debug message");
    QCOMPARE(spy.count(), 0);
}

void TestLogger::testErrorEmitsSignal()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().error("test error message");
    QCOMPARE(spy.count(), 1);
}

void TestLogger::testInfoMessageContent()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().info("hello world");
    QCOMPARE(spy.at(0).at(0).toString(), QString("hello world"));
}

void TestLogger::testErrorMessageContent()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().error("something went wrong");
    QCOMPARE(spy.at(0).at(0).toString(), QString("something went wrong"));
}

void TestLogger::testMultipleInfoCalls()
{
    QSignalSpy spy(&Logger::instance(), &Logger::messageLogged);
    Logger::instance().info("first");
    Logger::instance().info("second");
    Logger::instance().info("third");
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(0).at(0).toString(), QString("first"));
    QCOMPARE(spy.at(1).at(0).toString(), QString("second"));
    QCOMPARE(spy.at(2).at(0).toString(), QString("third"));
}

QTEST_GUILESS_MAIN(TestLogger)
#include "tst_logger.moc"
