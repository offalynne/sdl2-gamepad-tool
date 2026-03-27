#ifndef SDLPOLLEVENT_H
#define SDLPOLLEVENT_H

#include <QObject>
#include <atomic>

#include "GamepadMapper.h"

typedef enum
{
    APPMODE_TEST,
    APPMODE_MAP,
} AppMode;

typedef enum
{
    CUSTOM_EVENT_NEXT = 0,
    CUSTOM_EVENT_PREVIOUS,
} CustomEvent;

class SDLPollEvent : public QObject
{
    Q_OBJECT

public:
    explicit SDLPollEvent(QObject *parent = nullptr);
    void requestWork();
    void abort();
    bool isWorking() const;
    bool isAborted() const;
    std::atomic<int> mode{APPMODE_TEST};
    std::atomic<SDL_JoystickID> joystickId{0};

signals:
    void workRequested();
    void buttonPressed(int index, bool pressed);
    void finished();
    void mappingReady(QString mapping);
    void joystickAdded(int index);
    void joystickRemoved(int index);

public slots:
    void doWork();

private:
    std::atomic<bool> m_abort{false};
    std::atomic<bool> m_working{false};
    GamepadMapper m_mapper;
};

#endif // SDLPOLLEVENT_H
