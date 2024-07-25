#pragma once

#include <cstdint>

namespace wdvd {

bool init();
void deinit();

bool reset();
bool isInserted();
bool readDiskID();
bool unencryptedRead(void *buf, std::uint32_t len, std::uint32_t offset);
bool read(void *buf, std::uint32_t len, std::uint32_t offset);
bool openPartition(std::uint32_t offset);
bool closePartition();

}  // namespace wdvd
