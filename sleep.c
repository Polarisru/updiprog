#include "sleep.h"

void msleep(uint32_t msec)
{
  #ifdef __MINGW32__
	SleepEx(msec, false);
  #endif // __MINGW32__
  #if defined(__APPLE__) || defined(__linux)
  usleep(msec*1000);
  #endif // __linux
}

