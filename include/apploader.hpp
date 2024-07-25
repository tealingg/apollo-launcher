#pragma once

using entrypoint_t = void (*)();
extern entrypoint_t entrypoint;

namespace apploader {

void *loadAndRun(void *);

}  // namespace apploader
