#include "mainwindow.h"
#include <QApplication>
#include <QEventLoop>
#include <cstdio>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("GeneralArcade");
    a.setApplicationName("SDL2 Gamepad Tool");
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_JoystickEventState(SDL_ENABLE);
    MainWindow w;
    w.show();
    int ret = a.exec();
    SDL_Quit();
    return ret;
}
