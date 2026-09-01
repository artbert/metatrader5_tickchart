// main.cpp : Defines the entry point for the DLL application.
#include <windows.h>
#include "window_module.hpp"
#include "mt5_tick_chart_lib.hpp"

#define EXT extern "C" __declspec(dllexport)

HINSTANCE moduleInstance;
DWORD dwThreadId;
EXT bool __stdcall SetSymbolParameters(SYMBOLSETS &sets, char *date, int dateChars)
{
   return (chartModule.SetSymbolParameters(sets, date, dateChars));
}
EXT bool __stdcall Initialize(long long parent, wchar_t *dataPath, wchar_t *appTitle)
{
   MT5ParentChart = (HWND)parent;
   int i = 0;
   while (i < MAX_PATH && dataPath[i] != L'\0')
   {
      fullDataPath[i] = dataPath[i];
      ++i;
   }
   if (i == MAX_PATH)
      i = MAX_PATH - 1;
   fullDataPath[i] = L'\0';

   i = 0;
   while (i < MAX_LOADSTRING && appTitle[i] != L'\0')
   {
      szTitle[i] = appTitle[i];
      ++i;
   }
   if (i == MAX_LOADSTRING)
      i = MAX_LOADSTRING - 1;
   szTitle[i] = L'\0';

   if (threadHandle == nullptr)
   {
      try
      {
         threadHandle = CreateThread(nullptr, 0, MainModuleThreadFunction, (LPVOID)moduleInstance, 0, &dwThreadId);
      }
      catch (int e)
      {
         CHAR test[100];
         _i64toa_s((long long)e, test, 100, 10);
         MessageBoxA(nullptr, test, "INFO", MB_OK);
      }
   }
   else // Already initialized
   {
      PostMessage(MT5ParentChart, WM_LBUTTONUP, 0, -MODULE_INITIALIZED);
   }

   if (threadHandle != nullptr)
      return (true);
   else
      return (false);
}
EXT void __stdcall SignedLevelsAppend(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update)
{
   chartModule.AppendLevels(levels, levelsSize, descriptions, descrSize, update);
}
EXT bool __stdcall TickDataLoaded(MqlTick tckArray[], int arrSize, char *date, int dateChars)
{
   return (chartModule.TickDataLoaded(tckArray, arrSize, date, dateChars));
}
EXT bool __stdcall TickDataPartialLoaded(MqlTick tckArray[], int arrSize)
{
   return (chartModule.TickDataPartialLoaded(tckArray, arrSize));
}
EXT bool __stdcall NewRatesLoaded(MqlTick tckArray[], int arrSize)
{
   return (chartModule.NewRatesLoaded(tckArray, arrSize));
}
EXT bool __stdcall OnNewTick(MqlTick &lastTick)
{
   return (chartModule.OnNewTick(lastTick));
}
EXT void __stdcall TransactionsPointsAppend(const long transactions[][4], char (*descriptions)[64], const int size, const bool update)
{
   chartModule.AppendTransactionsPoints(transactions, descriptions, size, update);
}
EXT bool __stdcall GetSignedLevelData(double &levelValue, char *levelText, int arrSize)
{
   return (chartModule.GetSignedLevelData(levelValue, levelText, arrSize));
}
EXT bool __stdcall GetTimeAndPriceData(double &priceValue, time_t &timeValue)
{
   return (chartModule.GetTimeAndPriceData(priceValue, timeValue));
}
EXT bool __stdcall GetRangeValData(double &upRangeVal, double &downRangeVal)
{
   return (chartModule.GetRangeValData(upRangeVal, downRangeVal));
}
EXT bool __stdcall RangeLineDragged(double &newValue, int index)
{
   return (chartModule.RangeLineDragged(newValue, index));
}
EXT bool __stdcall TimeSepVLineDragged(time_t &newValue)
{
   return (chartModule.TimeSepVLineDragged(newValue));
}

EXT bool __stdcall ModuleDestroy()
{
   if (mainWindowHandle != nullptr)
      PostMessageA(mainWindowHandle, WM_CLOSE, 0, 0);
   return (true);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      moduleInstance = hModule;
      break;
   case DLL_THREAD_ATTACH:
      break;
   case DLL_THREAD_DETACH:
      break;
   case DLL_PROCESS_DETACH:
      if (mainWindowHandle != nullptr)
         PostMessageA(mainWindowHandle, WM_CLOSE, 0, 0);
      Sleep(1000);
      break;
   }
   return TRUE;
}