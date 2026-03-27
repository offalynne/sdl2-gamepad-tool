#include "Logger.h"
#include <cstdio>

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
}

void Logger::info(const QString& message)
{
    fprintf(stdout, "[LOG] %s\n", message.toUtf8().constData());
    fflush(stdout);
    emit messageLogged(message);
}

void Logger::debug(const QString& message)
{
    fprintf(stdout, "[DEBUG] %s\n", message.toUtf8().constData());
    fflush(stdout);
}

void Logger::error(const QString& message)
{
    fprintf(stderr, "[ERROR] %s\n", message.toUtf8().constData());
    fflush(stderr);
    emit messageLogged(message);
}
