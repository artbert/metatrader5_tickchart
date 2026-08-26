#pragma once

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#define DLL_CALL __stdcall
#else
#define DLL_EXPORT
#define DLL_CALL
#endif

// Exported functions
DLL_EXPORT bool DLL_CALL SetSymbolParameters(SYMBOLSETS &sets, char *date, int dateChars);
DLL_EXPORT bool DLL_CALL Initialize(long long parent, wchar_t *dataPath, wchar_t *appTitle);
DLL_EXPORT void DLL_CALL SignedLevelsAppend(const double levels[], const int levelsSize,
                                            char (*descriptions)[100], const int descrSize,
                                            const bool update);
DLL_EXPORT bool DLL_CALL TickDataLoaded(MqlTick tckArray[], int arrSize, char *date, int dateChars);
DLL_EXPORT bool DLL_CALL TickDataPartialLoaded(MqlTick tckArray[], int arrSize);
DLL_EXPORT bool DLL_CALL NewRatesLoaded(MqlTick tckArray[], int arrSize);
DLL_EXPORT bool DLL_CALL OnNewTick(MqlTick &lastTick);
DLL_EXPORT void DLL_CALL TransactionsPointsAppend(const long transactions[][4],
                                                  char (*descriptions)[64],
                                                  const int size, const bool update);
DLL_EXPORT bool DLL_CALL GetSignedLevelData(double &levelValue, char *levelText, int arrSize);
DLL_EXPORT bool DLL_CALL GetTimeAndPriceData(double &priceValue, time_t &timeValue);
DLL_EXPORT bool DLL_CALL GetRangeValData(double &upRangeVal, double &downRangeVal);
DLL_EXPORT bool DLL_CALL RangeLineDragged(double &newValue, int index);
DLL_EXPORT bool DLL_CALL TimeSepVLineDragged(time_t &newValue);
DLL_EXPORT bool DLL_CALL ModuleDestroy();