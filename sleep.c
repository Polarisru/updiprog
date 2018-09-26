#include "sleep.h"

void msleep(uint32_t msec)
{
  #ifdef __MINGW32__
	//DWORD startTime = GetTickCount();
	//while(GetTickCount() < (startTime + msec));
	SleepEx(msec, false);
  #endif // __MINGW32__
  #ifdef __linux
  usleep(msec*1000);
  #endif // __linux
}

