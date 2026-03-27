#ifndef JOYSTICKENUMERATOR_H
#define JOYSTICKENUMERATOR_H

#include <vector>
#include <QString>

#define SDL_MAIN_HANDLED
#include "SDL.h"

struct JoystickInfo {
    int index;
    QString name;
    QString guid;
    bool isGameController;
};

// Enumerate all connected joysticks. Opens and closes each joystick
// internally — the returned data is a snapshot requiring no cleanup.
std::vector<JoystickInfo> enumerateJoysticks();

#endif // JOYSTICKENUMERATOR_H
