#ifndef GAMEPADMAPPER_H
#define GAMEPADMAPPER_H

#include <functional>
#include <vector>
#include <QString>

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_gamecontroller.h"

// Axis binding identifiers for split-axis mapping
enum
{
    SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_BINDING_AXIS_MAX,
};

#define BINDING_COUNT (SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_MAX)

struct SDL_GameControllerExtendedBind
{
    SDL_GameControllerBindType bindType;
    union
    {
        int button;

        struct {
            int axis;
            int axis_min;
            int axis_max;
        } axis;

        struct {
            int hat;
            int hat_mask;
        } hat;

    } value;

    SDL_bool committed;
};

struct AxisState
{
    SDL_bool m_bMoving;
    int m_nLastValue;
    int m_nStartingValue;
    int m_nFarthestValue;
};

class GamepadMapper
{
public:
    // Named constants replacing magic numbers
    static constexpr int AXIS_COMMIT_DISTANCE = 16000;
    static constexpr int AXIS_RETURN_DISTANCE = 10000;
    static constexpr Uint32 PENDING_ADVANCE_DELAY_MS = 100;

    static const int BINDING_ORDER[BINDING_COUNT];

    using ButtonPressCallback = std::function<void(int, bool)>;

    GamepadMapper();

    void setButtonPressCallback(ButtonPressCallback callback);

    // Initialize mapping state for a new mapping session
    void reset(SDL_Joystick *joystick);

    // Binding navigation
    void setCurrentBinding(int iBinding);
    int currentBinding() const;
    bool isComplete() const;

    // Process input events
    void configureBinding(const SDL_GameControllerExtendedBind *pBinding);
    void processAxisMotion(int axis, int value, SDL_Joystick *joystick);

    // Check if pending advance timer has elapsed; advances binding if so
    bool checkPendingAdvance();

    // Generate the SDL mapping string after all bindings are complete
    QString generateMappingString(SDL_JoystickID joystickId);

    void clearAxisState();

    // Exposed for testing
    SDL_bool bMergeAxisBindings(int iIndex);
    SDL_bool bBindingContainsBinding(const SDL_GameControllerExtendedBind *pBindingA,
                                     const SDL_GameControllerExtendedBind *pBindingB);

private:
    static int standardizeAxisValue(int nValue);

    SDL_GameControllerExtendedBind m_arrBindings[BINDING_COUNT];
    int m_iCurrentBinding;
    Uint32 m_unPendingAdvanceTime;
    SDL_bool m_bBindingComplete;
    std::vector<AxisState> m_arrAxisState;

    ButtonPressCallback m_buttonPressCallback;
};

#endif // GAMEPADMAPPER_H
