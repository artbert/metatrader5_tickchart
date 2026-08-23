#include "stdafx.h"
#include "Resource.h"

long long ProccessorFrequency=1;

long long GetMicrosecondCount()
{
   /* LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
      LARGE_INTEGER Frequency;
      QueryPerformanceFrequency(&Frequency);*/

   //QueryPerformanceCounter(&StartingTime);
   //Code...
   //QueryPerformanceCounter(&EndingTime);

   //ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
   //long long interval = static_cast<long long>(ElapsedMicroseconds.QuadPart*1000000 / (Frequency.QuadPart));

   LARGE_INTEGER Tm;
   QueryPerformanceCounter(&Tm);

   return (static_cast<long long>(Tm.QuadPart*1000000) / ProccessorFrequency);
}