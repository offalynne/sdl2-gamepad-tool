#include <QtTest>
#include "sdlpollevent.h"
#include "GamepadMapper.h"

class TestSDLPollEvent : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();
    void testRequestWorkSetsWorking();
    void testAbortWhenWorking();
    void testAbortWhenNotWorking();
    void testRequestWorkResetsAbort();
    void cleanupTestCase();
};

void TestSDLPollEvent::initTestCase()
{
}

void TestSDLPollEvent::testInitialState()
{
    SDLPollEvent poller;
    QVERIFY(!poller.isWorking());
    QVERIFY(!poller.isAborted());
    QCOMPARE(poller.joystickId, 0);
}

void TestSDLPollEvent::testRequestWorkSetsWorking()
{
    SDLPollEvent poller;
    QSignalSpy spy(&poller, &SDLPollEvent::workRequested);

    poller.requestWork();

    QVERIFY(poller.isWorking());
    QVERIFY(!poller.isAborted());
    QCOMPARE(spy.count(), 1);
}

void TestSDLPollEvent::testAbortWhenWorking()
{
    SDLPollEvent poller;
    poller.requestWork();
    QVERIFY(poller.isWorking());

    poller.abort();
    QVERIFY(poller.isAborted());
}

void TestSDLPollEvent::testAbortWhenNotWorking()
{
    SDLPollEvent poller;
    QVERIFY(!poller.isWorking());

    poller.abort();
    QVERIFY(!poller.isAborted());
}

void TestSDLPollEvent::testRequestWorkResetsAbort()
{
    SDLPollEvent poller;
    poller.requestWork();
    poller.abort();
    QVERIFY(poller.isAborted());

    poller.requestWork();
    QVERIFY(!poller.isAborted());
    QVERIFY(poller.isWorking());
}

void TestSDLPollEvent::cleanupTestCase()
{
    SDL_Quit();
}

QTEST_MAIN(TestSDLPollEvent)
#include "tst_sdlpollevent.moc"
