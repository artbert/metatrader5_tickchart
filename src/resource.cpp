#include <windows.h>
#include "resource.h"

long long ProccessorFrequency = 1;

long long GetMicrosecondCount()
{
   LARGE_INTEGER Tm;
   QueryPerformanceCounter(&Tm);

   return (static_cast<long long>(Tm.QuadPart * 1000000) / ProccessorFrequency);
}