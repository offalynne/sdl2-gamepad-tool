#include "sdlpollevent.h"
#include "Logger.h"

#include <QThread>

static constexpr double AXIS_MAX_VALUE = 32767.0;
static constexpr double AXIS_DISPLAY_THRESHOLD = 0.75;

SDLPollEvent::SDLPollEvent(QObject *parent) :
    QObject(parent),
    joystickId(0)
{
    m_mapper.setButtonPressCallback([this](int index, bool pressed) {
        emit buttonPressed(index, pressed);
    });
}

void SDLPollEvent::requestWork()
{
    m_working.store(true);
    m_abort.store(false);
    Logger::instance().debug(QString("Request worker start in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));

    emit workRequested();
}

void SDLPollEvent::abort()
{
    if (m_working.load()) {
        m_abort.store(true);
        Logger::instance().debug(QString("Request worker aborting in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));
    }
}

bool SDLPollEvent::isWorking() const
{
    return m_working.load();
}

bool SDLPollEvent::isAborted() const
{
    return m_abort.load();
}

void SDLPollEvent::doWork()
{
    Logger::instance().debug(QString("Starting worker process in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));
    SDL_Event ev;
    if (mode == APPMODE_TEST) {
        while (true) {
            if (m_abort.load()) {
                Logger::instance().debug(QString("Aborting worker process in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));
                break;
            }

            while (SDL_PollEvent(&ev)) {
                Logger::instance().debug(QStringLiteral("First SDL loop getting events"));
                switch (ev.type) {
                case SDL_JOYDEVICEADDED:
                    emit joystickAdded(ev.jdevice.which);
                    break;
                case SDL_JOYDEVICEREMOVED:
                    emit joystickRemoved(ev.jdevice.which);
                    break;
                case SDL_CONTROLLERBUTTONUP:
                    emit buttonPressed(ev.cbutton.button, false);
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    emit buttonPressed(ev.cbutton.button, true);
                    break;
                case SDL_CONTROLLERAXISMOTION:
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                        if (ev.caxis.value > 0) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else if (ev.caxis.value < 0)  {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE, false);
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE, false);
                        }
                    }
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                        if (ev.caxis.value > 0) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else if (ev.caxis.value < 0)  {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE, false);
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE, false);
                        }
                    }
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                        if (ev.caxis.value > 0) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else if (ev.caxis.value < 0)  {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE, false);
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE, false);
                        }
                    }
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                        if (ev.caxis.value > 0) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else if (ev.caxis.value < 0)  {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                        } else {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE, false);
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE, false);
                        }
                    }
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                    }
                    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                            emit buttonPressed(SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT, (abs(ev.caxis.value) / AXIS_MAX_VALUE) >= AXIS_DISPLAY_THRESHOLD);
                    }
                    break;
                default:
                    break;
                }
            }

            SDL_Delay(15);
        }
    } else {
        SDL_Event event = {0};
        SDL_JoystickID nJoystickID = joystickId;
        Logger::instance().debug(QString("m_iCurrentBinding %1").arg(m_mapper.currentBinding()));

        SDL_Joystick *joystick = SDL_JoystickFromInstanceID(nJoystickID);
        if (!joystick) {
            Logger::instance().debug(QStringLiteral("[WARNING] Joystick disconnected before mapping could start"));
            m_working.store(false);
            emit finished();
            return;
        }
        m_mapper.reset(joystick);

        bool done = false;
        while (!done && !m_mapper.isComplete()) {
            if (m_abort.load()) {
                emit buttonPressed(m_mapper.currentBinding(), false);
                Logger::instance().debug(QString("Aborting worker process in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));
                break;
            }
            while (!done && SDL_PollEvent(&event) > 0) {
                if (m_abort.load()) {
                    emit buttonPressed(m_mapper.currentBinding(), false);
                    break;
                }
                Logger::instance().debug(QString("Second SDL loop getting events %1").arg(event.type));
                switch (event.type) {
                case SDL_JOYDEVICEREMOVED:
                    if (event.jdevice.which == nJoystickID) {
                        Logger::instance().debug(QStringLiteral("[WARNING] Device disconnected during mapping!"));
                        done = true;
                    }
                    break;
                case SDL_JOYAXISMOTION:
                    if (event.jaxis.which == nJoystickID) {
                        m_mapper.processAxisMotion(event.jaxis.axis, event.jaxis.value, joystick);
                    }
                    break;
                case SDL_JOYHATMOTION:
                    if (event.jhat.which == nJoystickID) {
                        if (event.jhat.value != SDL_HAT_CENTERED) {
                            SDL_GameControllerExtendedBind binding;
                            SDL_zero(binding);
                            binding.bindType = SDL_CONTROLLER_BINDTYPE_HAT;
                            binding.value.hat.hat = event.jhat.hat;
                            binding.value.hat.hat_mask = event.jhat.value;
                            binding.committed = SDL_TRUE;
                            m_mapper.configureBinding(&binding);
                        }
                    }
                    break;
                case SDL_JOYBUTTONDOWN:
                    if (event.jbutton.which == nJoystickID) {
                        SDL_GameControllerExtendedBind binding;
                        SDL_zero(binding);
                        binding.bindType = SDL_CONTROLLER_BINDTYPE_BUTTON;
                        binding.value.button = event.jbutton.button;
                        binding.committed = SDL_TRUE;
                        m_mapper.configureBinding(&binding);
                    }
                    break;
                case SDL_USEREVENT:
                     if (event.user.code == (CUSTOM_EVENT_NEXT)) {
                          m_mapper.setCurrentBinding(m_mapper.currentBinding() + 1);
                     } else if (event.user.code == CUSTOM_EVENT_PREVIOUS) {
                          m_mapper.setCurrentBinding(m_mapper.currentBinding() - 1);
                    }
                    break;
                case SDL_QUIT:
                    done = true;
                    break;
                default:
                    break;
                }
            }

            SDL_Delay(15);

            m_mapper.checkPendingAdvance();
        }

        if (m_mapper.isComplete()) {
            QString mapping = m_mapper.generateMappingString(nJoystickID);
            emit buttonPressed(m_mapper.currentBinding(), false);
            emit mappingReady(mapping);
        }
        m_mapper.clearAxisState();
    }

    m_working.store(false);

    Logger::instance().debug(QString("Worker process finished in Thread %1").arg(reinterpret_cast<quintptr>(thread()->currentThreadId())));

    emit finished();
}
