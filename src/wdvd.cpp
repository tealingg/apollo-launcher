#include "wdvd.hpp"

#include <ogcsys.h>

namespace wdvd {

static int fd = -1;

bool init() {
    fd = IOS_Open("/dev/di", 0);
    if (fd < 0) {
        return false;
    }

    return true;
}

void deinit() {
    if (fd >= 0) {
        IOS_Close(fd);
        fd = -1;
    }
}

bool reset() {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];

    inbuf[0] = 0x8A << 24;
    inbuf[1] = 1;

    std::uint32_t ret = IOS_Ioctl(fd, 0x8A, inbuf, sizeof(inbuf), nullptr, 0);

    return ret == 1;
}

bool isInserted() {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];
    alignas(0x20) std::uint32_t outbuf[8];

    inbuf[0] = 0x88 << 24;

    std::uint32_t ret =
        IOS_Ioctl(fd, 0x88, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));

    return ret == 1 && outbuf[0] == 2;
}

bool readDiskID() {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];

    inbuf[0] = 0x70 << 24;

    std::uint32_t ret = IOS_Ioctl(fd, 0x70, inbuf, sizeof(inbuf),
                                  reinterpret_cast<void *>(0x80000000), 0x20);

    return ret == 1;
}

bool unencryptedRead(void *buf, std::uint32_t len, std::uint32_t offset) {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];

    inbuf[0] = 0x8D << 24;
    inbuf[1] = len;
    inbuf[2] = offset >> 2;

    std::uint32_t ret = IOS_Ioctl(fd, 0x8D, inbuf, sizeof(inbuf), buf, len);

    return ret == 1;
}

bool read(void *buf, std::uint32_t len, std::uint32_t offset) {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];

    inbuf[0] = 0x71 << 24;
    inbuf[1] = len;
    inbuf[2] = offset >> 2;

    std::uint32_t ret = IOS_Ioctl(fd, 0x71, inbuf, sizeof(inbuf), buf, len);

    return ret == 1;
}

bool openPartition(std::uint32_t offset) {
    if (fd < 0) {
        return false;
    }

    wdvd::closePartition();

    alignas(0x20) std::uint32_t inbuf[8];
    alignas(0x20) std::uint32_t outbuf[8];
    alignas(0x20) std::uint8_t tmd[0x49e4];
    alignas(0x20) ioctlv pairs[5];

    inbuf[0] = 0x8B << 24;
    inbuf[1] = offset >> 2;

    pairs[0].data = inbuf;
    pairs[0].len = sizeof(inbuf);
    pairs[1].data = nullptr;
    pairs[1].len = 0;
    pairs[2].data = nullptr;
    pairs[2].len = 0;
    pairs[3].data = tmd;
    pairs[3].len = sizeof(tmd);
    pairs[4].data = outbuf;
    pairs[4].len = sizeof(outbuf);

    std::uint32_t ret = IOS_Ioctlv(fd, 0x8B, 3, 2, pairs);

    return ret == 1;
}

bool closePartition() {
    if (fd < 0) {
        return false;
    }

    alignas(0x20) std::uint32_t inbuf[8];

    inbuf[0] = 0x8C << 24;

    std::uint32_t ret = IOS_Ioctl(fd, 0x8C, inbuf, sizeof(inbuf), nullptr, 0);

    return ret == 1;
}

}  // namespace wdvd
