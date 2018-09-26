#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>
#include <stdbool.h>
#ifdef __MINGW32__
#include <windows.h>
#include <unistd.h>
#endif // __MINGW32__
#ifdef __linux
#include <unistd.h>
#endif // __linux

extern void msleep(uint32_t usec);

#endif
