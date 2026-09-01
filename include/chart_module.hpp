#include "resource.h"
#include "tick_chart.hpp"
#include "bar_chart.hpp"
#include <mmsystem.h>
#include <gdiplus.h>

class CTickChartModule
{
private:
  TCHMODSET *appSets{};
  HWND hWnd{};
  HWND rootWnd;
  bool readyToUse;

  HWND tChHWnd, bChHWnd;
  HWND TerminalParentChartHWnd;

  HPEN m_simplePen{}, old_pen{};
  __int64 *timesTabInMs;
  double *bidsTab, *asksTab;
  int dataSize;

  CTickChart tickChart1;
  CBarChart barChart1;
  bool isCalendarDataRead;
  int seriesIndex, chartSearchIndex;
  double upRangeLineValue, downRangeLineValue;
  int barChartTickSizeCounter;

  double barChartHigh{};
  double barChartLow{};
  int milisecondTimerInterval;
  bool isTimerOn;
  bool pointerLabelVisible{};
  int doubleSignificantPlaces{};

  int collectedDataSize{};
  int noOfSecondsForCalc;

  int *realTempoValsTab;

  int PipsDivider;
  double PipsDividerMultiplier;
  bool DarkMode;
  double _Point;
  int _Digits;
  double _DigitsMultiplier;

  bool ExpandDateRange;
  __int64 TimeSepVLine;
  bool ExcludePremarketData;
  bool EnableSpeedStats;

  char TOOLTIPLABELCONTENT[1000]{};

  MqlTick last_tick{};
  ULONG_PTR m_gdiplusToken;

public:
  CTickChartModule(void);
  ~CTickChartModule(void);

  int OnTimer();

  bool Initialize(HWND terminalParent, HWND rootWindow, HWND tickChartHWnd, HWND barChartHWnd, HWND toolBoxHWnd);
  void ModuleDestroy();
  void PassSettingsObjRef(TCHMODSET *ref);
  void LoadFromServerBtnClicked();
  void LoadFromServerPartialBtnClicked();
  void ChartAutoScrollChBxChanged();
  void StepForwardBtnClicked();
  void StepBackwardBtnClicked(bool rewind = false, bool forceVScaleUpdate = false);
  void MeasureChBxChanged();
  void ReplayBtnClicked();
  void SetRangeBtnClicked();
  void SetBarTickSizeBtnClicked();
  void MoveRangeUpBtnClicked();
  void MoveRangeDownBtnClicked();
  void SignedLevelsEditChBxChanged();
  void AddSignedLevelBtnClicked();
  void DeleteSignedLevelsBtnClicked();

  void TickChartTTipChanged(int posX, int posY, short LBUTTON_state, wchar_t *defaultRootTitle);
  void BarChartTTipChanged(int posX, int posY, short LBUTTON_state, wchar_t *defaultRootTitle);

  void TickChartZoomChanged();
  void BarChartCandleWidthChanged();
  void TickChartTimeSepChanged();
  void BarChartTimeSepChanged();
  void FreezeRanges();
  void ResetCharts();
  void RefreshTickChartWindow();
  void RefreshTickChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
  void RefreshBarChartWindow();
  void RefreshBarChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
  void AskLineVisChanged();
  void BidLineVisChanged();
  void SignedLevelsVisChanged();
  void AutoMovingRangeChanged();
  void MProfileAskVisChanged();
  void MProfileBidVisChanged();
  void TickChartVisChanged();
  void BarChartVisChanged();
  void TimeParameterVisChanged();
  void CumulativeAskVisChanged();
  void CumulativeBidVisChanged();
  void DistanceVisChanged();
  void RoadVisChanged();
  void TicksArrivedVisChanged();
  void EventsOnTickChartVisChanged();
  void EventsOnBarChartVisChanged();
  void OrdersVisChanged();
  void ColorTimeParamChanged();
  void ZoomTimeParamChanged();
  bool SaveTicksClicked(LPCTSTR pszFileName);
  bool PrintScreenClicked(LPCTSTR pszFileName);
  void RealTempoChanged();
  void TimerIntervalChanged();

  bool ReadTickDataFromFile(LPCTSTR pszFileName);

  void AppendLevels(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update);
  void AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size, const bool update);
  bool GetSignedLevelData(double &levelValue, char *levelText, int arrSize);
  bool GetRangeValData(double &upRangeVal, double &downRangeVal);
  bool RangeLineDragged(double &newValue, int index);
  bool TimeSepVLineDragged(time_t &newValue);
  bool SetSymbolParameters(SYMBOLSETS &sets, char *date, int dateChars);
  bool TickDataLoaded(MqlTick tckArray[], int arrSize, char *date, int dateChars);
  bool OnNewTick(MqlTick &lastTick);
  bool TickDataPartialLoaded(MqlTick tckArray[], int arrSize);
  bool NewRatesLoaded(MqlTick tckArray[], int arrSize);
  bool UpdateCharts(byte mode = 0, bool forceVScaleUpdate = false);
  void AutoScrollUpdate(bool forceVScaleUpdate = false);
  bool GetTimeAndPriceData(double &priceValue, time_t &timeValue);

private:
  void ResetData();
  void RecalculateBarChart(int endInd, bool redraw);
  void UpdateBiggerBarsData(bool updateChart);
  void UpdateVisLevels(bool updateChart);
  int GetEncoderClsid(const WCHAR *format, CLSID *pClsid);
};