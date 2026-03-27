#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger& instance();

    void info(const QString& message);
    void debug(const QString& message);
    void error(const QString& message);

signals:
    void messageLogged(const QString& message);

private:
    explicit Logger(QObject *parent = nullptr);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H
