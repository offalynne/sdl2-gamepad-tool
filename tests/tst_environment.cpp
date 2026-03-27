#include <QtTest>
#include "Environment.h"

class TestEnvironment : public QObject
{
    Q_OBJECT

private slots:
    void factoryCreatesInstance()
    {
        auto env = Environment::create();
        QVERIFY(env != nullptr);
    }

#ifdef Q_OS_LINUX
    // The following tests seed state via qputenv()/qunsetenv() which modify
    // the process environment.  EnvironmentLinux::get() reads the process
    // environment (getenv), so this approach works.  On macOS and Windows the
    // backends read from launchctl / the registry respectively, so process-
    // level env vars would not be visible — guard these tests to Linux only.

    void getReturnsEmptyForUnsetVar()
    {
        auto env = Environment::create();
        QString result = env->get("GAMEPADTOOL_TEST_NONEXISTENT_VAR_12345");
        QCOMPARE(result, QString(""));
    }

    void getReturnsValueForSetVar()
    {
        // Set a process-level env var for testing get()
        qputenv("GAMEPADTOOL_TEST_VAR", "test_value_42");
        auto env = Environment::create();
        QString result = env->get("GAMEPADTOOL_TEST_VAR");
        QCOMPARE(result, QString("test_value_42"));
        qunsetenv("GAMEPADTOOL_TEST_VAR");
    }

    void getReturnsEmptyAfterProcessUnset()
    {
        qputenv("GAMEPADTOOL_TEST_VAR2", "some_value");
        auto env = Environment::create();
        QCOMPARE(env->get("GAMEPADTOOL_TEST_VAR2"), QString("some_value"));

        qunsetenv("GAMEPADTOOL_TEST_VAR2");
        QCOMPARE(env->get("GAMEPADTOOL_TEST_VAR2"), QString(""));
    }

    void multipleInstancesShareSameBackend()
    {
        qputenv("GAMEPADTOOL_TEST_VAR3", "shared");
        auto env1 = Environment::create();
        auto env2 = Environment::create();
        QCOMPARE(env1->get("GAMEPADTOOL_TEST_VAR3"), env2->get("GAMEPADTOOL_TEST_VAR3"));
        qunsetenv("GAMEPADTOOL_TEST_VAR3");
    }
#endif
};

QTEST_MAIN(TestEnvironment)
#include "tst_environment.moc"
