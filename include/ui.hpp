#pragma once

namespace ui {

enum class state {
    Starting,
    Waiting,
    Launching,
    Error,
    Launch,
};

extern state uiState;
extern const char *launchStatus;

bool init();
void loop();

void starting();
void waiting();
void launching();
void error();

}  // namespace ui
