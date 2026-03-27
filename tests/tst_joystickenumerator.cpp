#include <QtTest>
#include "JoystickEnumerator.h"

class TestJoystickEnumerator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testEnumerateReturnsVector();
    void testJoystickInfoFieldAssignment();
    void cleanupTestCase();
};

void TestJoystickEnumerator::initTestCase()
{
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
}

void TestJoystickEnumerator::testEnumerateReturnsVector()
{
    // Without physical hardware this returns an empty vector;
    // the important thing is it doesn't crash and returns a valid container.
    std::vector<JoystickInfo> joysticks = enumerateJoysticks();
    QCOMPARE(static_cast<int>(joysticks.size()), SDL_NumJoysticks());
}

void TestJoystickEnumerator::testJoystickInfoFieldAssignment()
{
    JoystickInfo info;
    info.index = 0;
    info.name = "Test Pad";
    info.guid = "03000000abcd0000efgh000000000000";
    info.isGameController = true;

    QCOMPARE(info.index, 0);
    QCOMPARE(info.name, QString("Test Pad"));
    QVERIFY(!info.guid.isEmpty());
    QVERIFY(info.isGameController);
}

void TestJoystickEnumerator::cleanupTestCase()
{
    SDL_Quit();
}

QTEST_MAIN(TestJoystickEnumerator)
#include "tst_joystickenumerator.moc"
