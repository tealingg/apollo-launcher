#include "apploader.hpp"

#include <ogcsys.h>
#include <string.h>
#include <unistd.h>

#include <cstdarg>
#include <cstdio>
#include <string>

#include "ui.hpp"
#include "wdvd.hpp"

namespace apploader {

struct partitionInfo_t {
    u32 count;
    u32 offset;
};

struct partition_t {
    u32 offset;
    u32 type;
};

using appreport_t = void (*)(const char *, ...);

using appinit_t = void (*)(appreport_t);
using appmain_t = s32 (*)(void **dst, u32 *size, u32 *offset);
using appfinish_t = entrypoint_t (*)(void);

using appentry_t = void (*)(appinit_t *, appmain_t *, appfinish_t *);

struct apploaderHeader_t {
    char revision[0x10];
    appentry_t appentry;
    u32 size;
    u32 trailer;
    u8 unk[0x20 - 0x1c];
};

void apploaderReport(const char *fmt, ...) {
    // if i ever get a usb gecko, then i will actually use this.

    /*va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);*/
}

void *loadAndRun(void *a1) {
    while (ui::uiState != ui::state::Launching) {
        sleep(1);
    }

    while (!wdvd::isInserted()) {
        ui::launchStatus = "Please insert a Mario Kart Wii disc. (1)";

        while (!wdvd::isInserted()) {
            sleep(1);
        }

        if (!wdvd::reset()) {
            ui::launchStatus = "Failed to reset DVD drive. (2)";
            ui::uiState = ui::state::Error;

            return nullptr;
        }

        if (wdvd::isInserted()) {
            break;
        }
    }

    ui::launchStatus = "Reading disc ID...";

    if (!wdvd::readDiskID()) {
        ui::launchStatus = "Failed to read disc ID.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    char discID[5] = {0};  // TODO: this way of checking the disc ID is ugly

    for (int i = 0; i < 4; i++) {
        discID[i] =
            reinterpret_cast<u32 *>(0x80000000)[0] >> (24 - i * 8) & 0xFF;
    }

    if (std::string(discID).rfind("RMC", 0) != 0) {
        ui::launchStatus = "Please insert a Mario Kart Wii disc. (2)";

        while (wdvd::isInserted()) {
            sleep(1);
        }

        return loadAndRun(a1);
    }

    ui::launchStatus = "Opening Game partition...";

    alignas(0x20) partitionInfo_t partitionInfo[4];

    if (!wdvd::unencryptedRead(partitionInfo, sizeof(partitionInfo),
                               0x00040000)) {
        ui::launchStatus = "Failed to read partition info.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    for (int i = 0; i < 4; i++) {
        int count = partitionInfo[i].count;
        int offset = partitionInfo[i].offset << 2;

        if (count == 0 || count > 4 || offset == 0) {
            continue;
        }

        alignas(0x20) partition_t partition[4];

        if (!wdvd::unencryptedRead(partition, sizeof(partition), offset)) {
            continue;
        }

        for (int j = 0; j < count; j++) {
            if (partition[j].type == 0) {
                if (!wdvd::openPartition(partition[j].offset << 2)) {
                    ui::launchStatus = "Failed to open the Game partition.";
                    ui::uiState = ui::state::Error;

                    return nullptr;
                }
            }
        }
    }

    ui::launchStatus = "Reading apploader...";

    alignas(0x20) apploaderHeader_t apploaderHeader;
    if (!wdvd::read(&apploaderHeader, sizeof(apploaderHeader), 0x2440)) {
        ui::launchStatus = "Failed to read apploader header.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    if (!wdvd::read(reinterpret_cast<void *>(0x81200000),
                    apploaderHeader.size + apploaderHeader.trailer, 0x2460)) {
        ui::launchStatus = "Failed to read apploader.";
        ui::uiState = ui::state::Error;

        return nullptr;
    }

    ICInvalidateRange(reinterpret_cast<void *>(0x81200000),
                      apploaderHeader.size + apploaderHeader.trailer);

    *reinterpret_cast<u32 *>(0x800000F0) = 0x01800000;
    *reinterpret_cast<u32 *>(0x80000028) = 0x01800000;
    *reinterpret_cast<u32 *>(0x800000F4) = 0;
    *reinterpret_cast<u32 *>(0x80000020) = 0x0D15EA5E;
    *reinterpret_cast<u32 *>(0x80000024) = 1;
    *reinterpret_cast<u32 *>(0x80000030) = 0;
    *reinterpret_cast<u32 *>(0x800000F8) = 0x0E7BE2C0;
    *reinterpret_cast<u32 *>(0x800000FC) = 0x2B73A840;
    *reinterpret_cast<u32 *>(0x80003184) = 0x80000000;
    memcpy(reinterpret_cast<u32 *>(0x80003180),
           reinterpret_cast<u32 *>(0x80000000), 4);
    DCFlushRange(reinterpret_cast<u32 *>(0x80000000), 0x3F00);

    ui::launchStatus = "Launching game...";

    appinit_t appinit;
    appmain_t appmain;
    appfinish_t appfinish;

    apploaderHeader.appentry(&appinit, &appmain, &appfinish);

    appinit(apploaderReport);
    ui::uiState = ui::state::Launch;

    void *dst;
    u32 size;
    u32 offset;
    while (appmain(&dst, &size, &offset)) {
        if (!wdvd::read(dst, size, offset << 2)) {
            ui::launchStatus = "Failed to read game into memory.";
            ui::uiState = ui::state::Error;

            return nullptr;
        }

        DCFlushRange(dst, size);
    }

    entrypoint = appfinish();

    return nullptr;
}

}  // namespace apploader
