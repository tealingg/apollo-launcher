#include "ui.hpp"

#include <grrlib.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include <cstdarg>
#include <cstdio>
#include <mutex>

#include "apploader.hpp"
#include "fonts/Roboto.hpp"

// TODO: when we create a github action, make sure we pass these in.

#ifndef GHA_SHA
#define GHA_SHA "fffffff"
#endif
#ifndef GHA_VER
#define GHA_VER "v0.0.0"
#endif
#ifndef GHA_TYPE
#define GHA_TYPE "dev"
#endif

namespace ui {

state uiState = state::Starting;
const char *launchStatus = "Please Wait...";

GRRLIB_ttfFont *roboto = nullptr;

void GRRLIB_PrintfTTF2(int x, int y, GRRLIB_ttfFont *font, u32 fontSize,
                       u32 color, const char *text, ...) {
    va_list args;
    va_start(args, text);

    char buffer[1024];
    std::vsprintf(buffer, text, args);

    GRRLIB_PrintfTTF(x, y, font, buffer, fontSize, color);

    va_end(args);
}

u32 getCenter(const char *text, u32 fontSize) {
    return (640 - GRRLIB_WidthTTF(roboto, text, fontSize)) / 2;
}

void printInCenter(const char *text, int y, u32 fontSize, u32 color) {
    GRRLIB_PrintfTTF(getCenter(text, fontSize), y, roboto, text, fontSize,
                     color);
}

bool init() {
    if (GRRLIB_Init() != 0) {
        return false;
    }

    roboto = GRRLIB_LoadTTF(Roboto_Regular, Roboto_Regular_size);

    if (roboto == nullptr) {
        return false;
    }

    return true;
}

void drawBackground() {
    GRRLIB_FillScreen(0x17171AFF);

    GRRLIB_PrintfTTF2(16, 16, roboto, 20, 0xFFFFFFFF,
                      "Apollo Launcher %s-%s (%s)", GHA_VER, GHA_TYPE, GHA_SHA);
}

void fadeIn() {
    for (int i = 255; i >= 0; i -= 15) {
        drawBackground();

        GRRLIB_Rectangle(0, 0, 640, 480, 0x0 | i, true);

        GRRLIB_Render();
    }
}

void fadeOut() {
    for (int i = 0; i <= 255; i += 15) {
        drawBackground();

        GRRLIB_Rectangle(0, 0, 640, 480, 0x0 | i, true);

        GRRLIB_Render();
    }
}

void loop() {
    // for now, i am hardcoding a wait for the dvd drive to spin up, as if the
    // launcher is too fast, freaky stuff happens
    // TODO: find a way to figure out how to wait for the dvd drive to spin up
    // TODO: also, maybe don't do this if we're on dolphin (no problem there)

    for (int i = 0; i < 300; i++) {
        GRRLIB_Render();  // i CANNOT believe i am relying on 60fps to wait
                          // instead of calling sleep()...
    }

    fadeIn();

    while (true) {
        drawBackground();

        switch (uiState) {
            case state::Starting:
                starting();
                break;
            case state::Waiting:
                waiting();
                break;
            case state::Launching:
                launching();
                break;
            case state::Error:
                error();
                break;
            default:
                break;
        }

        GRRLIB_Render();

        if (uiState == state::Launch) {
            fadeOut();

            VIDEO_SetBlack(TRUE);  // stop the screen from flashing. grrlib, why
                                   // can't you do this for me?
            VIDEO_Flush();

            GRRLIB_Exit();

            while (entrypoint == nullptr) {
                sleep(1);
            }

            break;
        }
    }
}

void starting() { printInCenter("Initialising...", 232, 16, 0xFFFFFFFF); }

void waiting() {
    printInCenter("Press A to launch.", 220, 16, 0xFFFFFFFF);
    printInCenter("Press HOME to exit.", 244, 16, 0xFFFFFFFF);

    WPAD_ScanPads();

    u32 pressed = WPAD_ButtonsDown(0);
    if (pressed & WPAD_BUTTON_A) {
        uiState = state::Launching;
    } else if (pressed & WPAD_BUTTON_HOME) {
        uiState = state::Launch;
    }
}

void launching() { printInCenter(launchStatus, 232, 16, 0xFFFFFFFF); }

void error() {
    printInCenter("An error occurred:", 212, 16, 0xFFFFFFFF);
    printInCenter(launchStatus, 232, 16, 0xFFFFFFFF);
    printInCenter("Press HOME to exit.", 256, 16, 0xFFFFFFFF);

    WPAD_ScanPads();

    u32 pressed = WPAD_ButtonsDown(0);
    if (pressed & WPAD_BUTTON_HOME) {
        uiState = state::Launch;
    }
}

}  // namespace ui
