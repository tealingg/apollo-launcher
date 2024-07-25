#include <fat.h>
#include <ogc/lwp.h>
#include <ogc/lwp_threads.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include <cstdint>
#include <cstdio>

#include "apploader.hpp"
#include "ui.hpp"
#include "wdvd.hpp"

entrypoint_t entrypoint = nullptr;

void *initSubsystems(void *) {
    if (WPAD_Init() != WPAD_ERR_NONE) {
        ui::launchStatus = "Failed to initialize Wiimote subsystem.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    if (!wdvd::init()) {
        ui::launchStatus = "Failed to initialize DVD drive.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    if (!wdvd::reset()) {
        ui::launchStatus = "Failed to reset DVD drive. (1)";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    ui::uiState = ui::state::Waiting;

    return nullptr;
}

int main(int argc, char *argv[]) {
    if (!ui::init()) {
        return -1;
    }

    lwp_t subsystemThread;
    lwp_t apploaderThread;

    LWP_CreateThread(&subsystemThread, initSubsystems, nullptr, nullptr, 0, 0);
    LWP_CreateThread(&apploaderThread, apploader::loadAndRun, nullptr, nullptr,
                     0, 0);

    ui::loop();

    if (entrypoint != nullptr) {
        wdvd::deinit();
        SYS_ProtectRange(SYS_PROTECTCHAN3, nullptr, 0, SYS_PROTECTRDWR);
        __MaskIrq(IM_MEMADDRESS);
        DCFlushRangeNoSync(reinterpret_cast<void *>(0x80000000), 0x01800000);
        ICFlashInvalidate();
        SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

        __lwp_thread_stopmultitasking(entrypoint);
    }

    // we get here, we're in trouble.

    return 0;
}
