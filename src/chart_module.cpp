#include "chart_module.hpp"
#include <algorithm>
#include <ctime>
#include <utility>

CTickChartModule::CTickChartModule() : rootWnd(nullptr), tChHWnd(nullptr), bChHWnd(nullptr), TerminalParentChartHWnd(nullptr), readyToUse(false), timesTabInMs(nullptr), bidsTab(nullptr), asksTab(nullptr), realTempoValsTab(nullptr), dataSize(0), isCalendarDataRead(false), seriesIndex(-1), chartSearchIndex(-1), upRangeLineValue(0), downRangeLineValue(0), barChartTickSizeCounter(0), milisecondTimerInterval(1000), isTimerOn(false), noOfSecondsForCalc(7), PipsDivider(10), PipsDividerMultiplier(0.1), DarkMode(true), symbol_point_size(0.00001), price_multiplier(POWER_OF_10[5]), symbol_digits(5), ExpandDateRange(true), ExcludePremarketData(true), EnableSpeedStats(false), TimeSepVLine(0), m_gdiplusToken(0)
{
}

CTickChartModule::~CTickChartModule()
{
   delete[] timesTabInMs;
   delete[] bidsTab;
   delete[] asksTab;
   delete[] realTempoValsTab;
   DeleteObject(m_simplePen);

   tick_chart.Destroy();
   bar_chart.Destroy();
}

bool CTickChartModule::Initialize(HWND terminalParent, HWND rootWindow, HWND tickChartHWnd, HWND barChartHWnd, HWND toolBoxHWnd)
{
   dataSize = 1000000;
   delete[] timesTabInMs;
   delete[] bidsTab;
   delete[] asksTab;
   delete[] realTempoValsTab;

   timesTabInMs = new __int64[dataSize];
   bidsTab = new double[dataSize];
   asksTab = new double[dataSize];
   realTempoValsTab = new int[dataSize];
   realTempoValsTab[0] = (*appSets).timerInterval;

   for (int i = 0; i < dataSize; i++)
   {
      timesTabInMs[i] = 0;
      bidsTab[i] = last_tick.bid;
      asksTab[i] = last_tick.ask;
   }
   NormalizationArgs args = {(last_tick.bid + last_tick.ask) / 2.0, symbol_digits};
   double midPrice = NormalizeDouble(args);
   args.value = midPrice + (100 * symbol_point_size);
   upRangeLineValue = NormalizeDouble(args);
   args.value = midPrice - (100 * symbol_point_size);
   downRangeLineValue = NormalizeDouble(args);

   TerminalParentChartHWnd = terminalParent;
   rootWnd = rootWindow;
   tChHWnd = tickChartHWnd;
   bChHWnd = barChartHWnd;
   hWnd = toolBoxHWnd;

   PipsDivider = 10;
   PipsDividerMultiplier = 0.1;
   tick_chart.Create(tickChartHWnd, 1100, 428, symbol_point_size, symbol_digits);
   bar_chart.Create(barChartHWnd, 1100, 300, symbol_point_size, symbol_digits);

   tick_chart.ShowScaleTop(false, false);
   tick_chart.ShowScaleRight(false, false);
   tick_chart.ShowScaleLeft(true, false);
   tick_chart.ShowScaleBottom(false, false);
   tick_chart.ShowLegend(false, false);
   tick_chart.ShowGrid(true, false);
   tick_chart.SetScaleDigits(symbol_digits);
   tick_chart.ShowDescriptors(false, false);
   tick_chart.SetPipsDivider(PipsDivider);
   doubleSignificantPlaces = (int)log10((double)PipsDivider);

   char charBuffer[50];
   sprintf_s(charBuffer, "%.*f", doubleSignificantPlaces, (abs(upRangeLineValue - downRangeLineValue) * price_multiplier) * PipsDividerMultiplier);

   size_t len = strlen(charBuffer);

   charBuffer[len] = ' ';
   charBuffer[len + 1] = 'p';
   charBuffer[len + 2] = 'i';
   charBuffer[len + 3] = 'p';
   charBuffer[len + 4] = 's';
   charBuffer[len + 5] = '\0';

   SetDlgItemTextA(hWnd, IDC_INFOLABEL, charBuffer);

   tick_chart.VScaleParams(upRangeLineValue, downRangeLineValue, 10, false);
   tick_chart.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);

   if (DarkMode)
   {
      tick_chart.ColorBackground(XRGB_gdi(0x00, 0x00, 0x00), false);
      tick_chart.ColorBorder(XRGB_gdi(0xFF, 0xFF, 0xFF) - tick_chart.ColorBorder(), false);
      tick_chart.ColorText(XRGB_gdi(0xFF, 0xFF, 0xFF) - tick_chart.ColorText(), false);
      tick_chart.ColorGrid(XRGB_gdi(0xFF, 0xFF, 0xFF) - tick_chart.ColorGrid(), false);
      tick_chart.TextColorSet(tick_chart.ColorText());
   }

   bar_chart.ShowScaleTop(false, false);
   bar_chart.ShowScaleRight(false, false);
   bar_chart.ShowScaleLeft(true, false);
   bar_chart.ShowScaleBottom(false, false);
   bar_chart.ShowLegend(false, false);
   bar_chart.ShowGrid(true, false);
   bar_chart.SetScaleDigits(symbol_digits);
   bar_chart.ShowDescriptors(false, false);
   bar_chart.SetPipsDivider(PipsDivider);

   if (DarkMode)
   {
      bar_chart.ColorBackground(XRGB_gdi(0x00, 0x00, 0x00), false);
      bar_chart.ColorBorder(XRGB_gdi(0xFF, 0xFF, 0xFF) - bar_chart.ColorBorder(), false);
      bar_chart.ColorText(XRGB_gdi(0xFF, 0xFF, 0xFF) - bar_chart.ColorText(), false);
      bar_chart.ColorGrid(XRGB_gdi(0xFF, 0xFF, 0xFF) - bar_chart.ColorGrid(), false);
   }

   tick_chart.CreatePixelFontSet(tick_chart.ColorBackground());
   bar_chart.CreatePixelFontSet(bar_chart.ColorBackground());

   m_simplePen = CreatePen(0, 1, tick_chart.ColorText());

   tick_chart.ChartVisibility(0, (int)(*appSets).askLineVis, false);
   tick_chart.ChartVisibility(1, (int)(*appSets).bidLineVis, false);
   tick_chart.ShowCumulativeDataBid((*appSets).cumulativeBidVis, false);
   tick_chart.ShowCumulativeDataAsk((*appSets).cumulativeAskVis, false);

   if (symbol_digits <= 5)
   {
      tick_chart.ShowMProfileDataAsk((*appSets).mProfileAskVis, false);
      tick_chart.ShowMProfileDataBid((*appSets).mProfileBidVis, false);
      bar_chart.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, false);
   }

   tick_chart.SetInterval((*appSets).tickChartTimeSep, false);
   bar_chart.SetInterval((*appSets).barChartTimeSep, false);

   bar_chart.SetBarChartBarWidth((*appSets).barChartCandleWidth, false);

   tick_chart.ShowTimeParameter((*appSets).timeParamVis, false);
   bar_chart.ShowTimeParameter((*appSets).timeParamVis, false);
   tick_chart.ShowMainPlot((*appSets).tickChartVis, false);
   bar_chart.ShowMainPlot((*appSets).barChartVis, false);

   tick_chart.ShowSignedLevels((*appSets).signedLevelsVis, false);
   bar_chart.ShowSignedLevels((*appSets).signedLevelsVis, false);
   tick_chart.SetTickSampleWidth((*appSets).tickChartZoom, false);
   tick_chart.ColorTimeParameter((*appSets).colorTimeParam, false);
   bar_chart.ColorTimeParameter((*appSets).colorTimeParam, false);
   tick_chart.ShowCalendarEvents((*appSets).eventsOnTickChartVis, false);
   bar_chart.ShowCalendarEvents((*appSets).eventsOnBarChartVis, false);
   bar_chart.SetBarChartTickSize((*appSets).barChartTickSize, false);

   tick_chart.FillSeries(last_tick.ask, last_tick.bid, 0, 0, 0, 0);
   bar_chart.FillSeries(last_tick.bid, last_tick.bid, last_tick.bid, last_tick.bid, 0, 0, false);

   UpdateVisLevels(false);
   UpdateBiggerBarsData(false);

   barChartHigh = DBL_MAX * -1.0;
   barChartLow = DBL_MAX;

   isTimerOn = false;

   SetDlgItemTextA(hWnd, IDC_DATELABEL, TOOLTIPLABELCONTENT);

   barChartTickSizeCounter = 0;
   chartSearchIndex = -1;
   seriesIndex = -1;

   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

   readyToUse = true;
   return (true);
}
void CTickChartModule::ModuleDestroy()
{
   readyToUse = false;
   if (timesTabInMs != nullptr)
   {
      delete[] timesTabInMs;
      timesTabInMs = nullptr;
   }
   if (bidsTab != nullptr)
   {
      delete[] bidsTab;
      bidsTab = nullptr;
   }
   if (asksTab != nullptr)
   {
      delete[] asksTab;
      asksTab = nullptr;
   }
   if (realTempoValsTab != nullptr)
   {
      delete[] realTempoValsTab;
      realTempoValsTab = nullptr;
   }

   rootWnd = nullptr;
   tChHWnd = nullptr;
   bChHWnd = nullptr;

   dataSize = 0;

   isCalendarDataRead = false;
   seriesIndex = -1;
   upRangeLineValue = 0;
   downRangeLineValue = 0;

   barChartTickSizeCounter = 0;

   milisecondTimerInterval = 1000;
   isTimerOn = false;
   noOfSecondsForCalc = 7;

   PipsDivider = 10;
   PipsDividerMultiplier = 0.1;
   DarkMode = true;
   symbol_point_size = 0.00001;
   symbol_digits = 5;

   ExpandDateRange = true;
   ExcludePremarketData = true;
   EnableSpeedStats = true;

   TimeSepVLine = 0;

   tick_chart.Destroy();
   bar_chart.Destroy();
   DeleteObject(m_simplePen);
   m_simplePen = nullptr;

   if (m_gdiplusToken != 0)
   {
      Gdiplus::GdiplusShutdown(m_gdiplusToken);
   }
}

void CTickChartModule::PassSettingsObjRef(TCHMODSET *ref)
{
   appSets = ref;
}
void CTickChartModule::LoadFromServerBtnClicked()
{
   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -LOAD_DATA_FROM_SERVER);
}
void CTickChartModule::LoadFromServerPartialBtnClicked()
{
   BOOL success = 0;
   uint minutes = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, 0);
   if (success != 0)
   {
      if (minutes > 0)
      {
         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, (WPARAM)minutes, -LOAD_DATA_FROM_SERVER_PARTIAL);
      }
   }
}
void CTickChartModule::ChartAutoScrollChBxChanged()
{
   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, WPARAM((*appSets).chartAutoScroll), -AUTO_SCROLL_CHANGED);
   if ((*appSets).chartAutoScroll && seriesIndex > -1)
   {
      AutoScrollUpdate();
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -CURRENT_TIME_PRICE_CHANGED);
   }
}
void CTickChartModule::StepForwardBtnClicked()
{
   if (seriesIndex == -1)
   {
      return;
   }

   if (!(*appSets).chartAutoScroll)
   {
      if (seriesIndex == chartSearchIndex)
      {
         return;
      }

      if (chartSearchIndex == -1)
      {
         tick_chart.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
         bar_chart.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);
         barChartTickSizeCounter = 0;
         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
      }
      int noOfTicks = (*appSets).tickOffsetValue;

      int tickCounter = 0;
      int i = chartSearchIndex + 1;

      bool updateBarChart = false;
      uint parameter = 0;
      for (; i <= seriesIndex; i++)
      {
         if (tickCounter >= noOfTicks)
         {
            break;
         }

         chartSearchIndex++;
         barChartTickSizeCounter++;

         int bidChange = 0;
         int askChange = 0;
         double wholeRoad = 0;
         short priceDistance = 0;
         short ticksElapsed = 0;
         if (i > 0)
         {
            parameter = (uint)(timesTabInMs[i] - timesTabInMs[i - 1]);

            if (bidsTab[i] > bidsTab[i - 1])
            {
               bidChange = 1;
            }
            else if (bidsTab[i] < bidsTab[i - 1])
            {
               bidChange = -1;
            }

            if (asksTab[i] > asksTab[i - 1])
            {
               askChange = 1;
            }
            else if (asksTab[i] < asksTab[i - 1])
            {
               askChange = -1;
            }

            if (EnableSpeedStats)
            {
               for (int j = i - 1; j > 0; j--)
               {
                  wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]) * price_multiplier;
                  if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
                  {
                     NormalizationArgs args = {(bidsTab[i - 1] - bidsTab[j]) * price_multiplier, 0};
                     priceDistance = (short)NormalizeDouble(args);
                     ticksElapsed = short(i - 1 - j);
                     break;
                  }
               }
            }
         }
         tick_chart.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)wholeRoad, priceDistance, ticksElapsed);

         barChartHigh = std::max(barChartHigh, bidsTab[i]);

         barChartLow = std::min(barChartLow, bidsTab[i]);

         if (barChartTickSizeCounter == (*appSets).barChartTickSize)
         {
            ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];

            bar_chart.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

            barChartHigh = DBL_MAX * -1.0;
            barChartLow = DBL_MAX;
            barChartTickSizeCounter = 0;
            updateBarChart = true;
         }

         tickCounter++;

         tick_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
         bar_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      }

      if ((*appSets).autoMovingRange && i > 0)
      {
         double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
         // instead of abs() for double
         (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
         double meanPrice = (bidsTab[i - 1] + asksTab[i - 1]) / 2.0;

         NormalizationArgs args = {meanPrice + halfDistance, symbol_digits};
         double newHPrice = NormalizeDouble(args);
         args.value = meanPrice - halfDistance;
         double newLPrice = NormalizeDouble(args);
         upRangeLineValue = newHPrice;
         downRangeLineValue = newLPrice;
         tick_chart.VScaleParams(newHPrice, newLPrice, 10, false);
         tick_chart.MoveMarketProfileRange(newLPrice, newHPrice);
         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }

      tick_chart.UpdateChart();

      bar_chart.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
      if (updateBarChart)
      {
         bar_chart.UpdateChart();
      }
      else
      {
         bar_chart.UpdateCurrentPriceLevel();
      }

      time_t _time = timesTabInMs[chartSearchIndex] / 1000;
      tm timeinfo = {.tm_sec = 0};
      localtime_s(&timeinfo, &_time);
      char date[20];
      strftime(date, sizeof(date), "%Y.%m.%d", &timeinfo);
      SetDlgItemTextA(hWnd, IDC_DATELABEL, date);

      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -CURRENT_TIME_PRICE_CHANGED);
   }
}
void CTickChartModule::StepBackwardBtnClicked(bool rewind, bool forceVScaleUpdate)
{
   if (seriesIndex == -1)
   {
      return;
   }

   if (!(*appSets).chartAutoScroll)
   {
      int upLimit = 0;
      if (rewind)
      {
         for (int n = 1; n < seriesIndex; n++)
         {
            if ((timesTabInMs[n] >= TimeSepVLine) && (timesTabInMs[n - 1] <= TimeSepVLine))
            {
               upLimit = n;
            }
         }
      }
      else
      {
         upLimit = chartSearchIndex - (*appSets).tickOffsetValue + 1;
      }

      upLimit = std::max(upLimit, 1);

      tick_chart.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
      bar_chart.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);

      double price1 = upRangeLineValue;
      double price2 = downRangeLineValue;
      tick_chart.VScaleParams(price1, price2, 10, false);
      tick_chart.MoveMarketProfileRange(price2, price1);

      barChartHigh = DBL_MAX * -1.0;
      barChartLow = DBL_MAX;
      barChartTickSizeCounter = 0;
      chartSearchIndex = -1;

      int i = 0;
      uint parameter = 0;
      for (; i < upLimit; i++)
      {
         chartSearchIndex++;
         barChartTickSizeCounter++;

         int bidChange = 0;
         int askChange = 0;

         double wholeRoad = 0;
         short priceDistance = 0;
         short ticksElapsed = 0;

         if (i > 0)
         {
            parameter = (uint)(timesTabInMs[i] - timesTabInMs[i - 1]);

            if (bidsTab[i] > bidsTab[i - 1])
            {
               bidChange = 1;
            }
            else if (bidsTab[i] < bidsTab[i - 1])
            {
               bidChange = -1;
            }

            if (asksTab[i] > asksTab[i - 1])
            {
               askChange = 1;
            }
            else if (asksTab[i] < asksTab[i - 1])
            {
               askChange = -1;
            }

            if (EnableSpeedStats)
            {
               for (int j = i - 1; j > 0; j--)
               {
                  wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]);
                  if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
                  {
                     NormalizationArgs args = {(bidsTab[i - 1] - bidsTab[j]) * price_multiplier, 0};
                     priceDistance = (short)NormalizeDouble(args);
                     ticksElapsed = short(i - 1 - j);
                     break;
                  }
               }
            }
         }
         tick_chart.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)(wholeRoad * price_multiplier), priceDistance, ticksElapsed);

         barChartHigh = std::max(barChartHigh, bidsTab[i]);

         barChartLow = std::min(barChartLow, bidsTab[i]);

         if (barChartTickSizeCounter == (*appSets).barChartTickSize)
         {
            ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
            bar_chart.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

            barChartHigh = DBL_MAX * -1.0;
            barChartLow = DBL_MAX;
            barChartTickSizeCounter = 0;
         }

         tick_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
         bar_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      }

      if ((*appSets).autoMovingRange && i > 0)
      {

         double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
         (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
         double meanPrice = (bidsTab[i - 1] + asksTab[i - 1]) / 2.0;

         NormalizationArgs args = {meanPrice + halfDistance, symbol_digits};
         double newHPrice = NormalizeDouble(args);
         args.value = meanPrice - halfDistance;
         double newLPrice = NormalizeDouble(args);
         upRangeLineValue = newHPrice;
         downRangeLineValue = newLPrice;
         tick_chart.VScaleParams(newHPrice, newLPrice, 10, false);
         tick_chart.MoveMarketProfileRange(newLPrice, newHPrice);
         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }

      tick_chart.UpdateChart(forceVScaleUpdate);

      bar_chart.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
      bar_chart.UpdateChart(forceVScaleUpdate);

      time_t _time = timesTabInMs[chartSearchIndex] / 1000;
      tm timeinfo = {.tm_sec = 0};
      localtime_s(&timeinfo, &_time);
      char date[20];
      strftime(date, sizeof(date), "%Y.%m.%d", &timeinfo);
      SetDlgItemTextA(hWnd, IDC_DATELABEL, date);

      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -CURRENT_TIME_PRICE_CHANGED);
   }
}
void CTickChartModule::MeasureChBxChanged()
{
   if ((*appSets).measurementTool)
   {
      tick_chart.PrepareAlphaBlend(tChHWnd);
      bar_chart.PrepareAlphaBlend(bChHWnd);
   }
   else
   {
      tick_chart.ReleaseAlphaBlend();
      bar_chart.ReleaseAlphaBlend();
   }
}
void CTickChartModule::TickChartTTipChanged(int posX, int posY, short LBUTTON_state, wchar_t *defaultRootTitle)
{
   if (seriesIndex > -1)
   {
      static bool defTitleChanged = false;
      static bool partialToRefresh = false;
      static RECT refreshRect = {0, 0, 0, 0};
      int startPoint = (int)tick_chart.GetDataAreaStartPoint();

      if ((uint)(posX - startPoint) < 1000 && (uint)(posY - 1) < 426)
      {

         int ttipTextPtr = 0;

         static time_t mouseDnTime = 0;
         static double mouseDnAsk = 0;
         static double mouseDnBid = 0;
         static int mouseDnPosX = 0;
         static int mouseDnPosY = 0;

         int shift = 0;

         if ((*appSets).tickChartZoom == 1)
         {
            shift = (posX - startPoint - 1000) * 2;
         }
         else if ((*appSets).tickChartZoom == 2)
         {
            shift = (posX - startPoint - 1000);
         }
         else
         {
            shift = (posX - startPoint - 1000) / 2;
         }

         int calculatedIndex = chartSearchIndex + (shift);

         calculatedIndex = std::max(calculatedIndex, 0);

         if (LBUTTON_state == 0)
         {
            if (partialToRefresh)
            {
               RefreshTickChartWindow(refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);
               partialToRefresh = false;
               refreshRect.bottom = 0;
               refreshRect.left = 0;
               refreshRect.right = 0;
               refreshRect.top = 0;
            }

            mouseDnAsk = asksTab[calculatedIndex];
            mouseDnBid = bidsTab[calculatedIndex];
            mouseDnTime = timesTabInMs[calculatedIndex];
            mouseDnPosX = posX;
            mouseDnPosY = posY;

            time_t t0 = timesTabInMs[calculatedIndex];
            time_t t1 = t0 / 1000;
            uint days = (((t1 * 1158050442) >> 32) + t1 * 49710) >> 32;
            ulong _sc = t1 - (days * 24 * 60 * 60);
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - (mn * 60));

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(t0 - (t1 * 1000));

            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';

            ttipTextPtr = 14 + sprintf_s(&TOOLTIPLABELCONTENT[14], 100, "%.*f", symbol_digits, mouseDnAsk);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';

            ttipTextPtr += 2;

            ttipTextPtr = ttipTextPtr + sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", symbol_digits, mouseDnBid);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';

            ttipTextPtr += 2;

            sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", doubleSignificantPlaces, ((mouseDnAsk - mouseDnBid) * price_multiplier) * PipsDividerMultiplier);

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;
         }
         else
         {
            ulong actualPreciseTime = timesTabInMs[calculatedIndex];
            ulong timeDiff = 0;
            if (std::cmp_greater_equal(actualPreciseTime, mouseDnTime))
            {
               timeDiff = actualPreciseTime - mouseDnTime;
            }
            else
            {
               timeDiff = mouseDnTime - actualPreciseTime;
            }

            int ticksCount = abs(mouseDnPosX - posX);
            if ((*appSets).tickChartZoom == 1)
            {
               ticksCount *= 2;
            }
            else if ((*appSets).tickChartZoom == 4)
            {
               ticksCount /= 2;
            }

            time_t td = timeDiff / 1000;
            uint days = (((td * 1158050442) >> 32) + td * 49710) >> 32;
            ulong _sc = td - (days * 24 * 60 * 60);
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - (mn * 60));

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(timeDiff - (td * 1000));
            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';
            TOOLTIPLABELCONTENT[14] = 'a';
            TOOLTIPLABELCONTENT[15] = ':';
            TOOLTIPLABELCONTENT[16] = ' ';

            ttipTextPtr = 17 + sprintf_s(&TOOLTIPLABELCONTENT[17], 100, "%.*f", doubleSignificantPlaces, ((asksTab[calculatedIndex] - mouseDnAsk) * price_multiplier) * PipsDividerMultiplier);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';
            TOOLTIPLABELCONTENT[ttipTextPtr + 2] = 'b';
            TOOLTIPLABELCONTENT[ttipTextPtr + 3] = ':';
            TOOLTIPLABELCONTENT[ttipTextPtr + 4] = ' ';

            ttipTextPtr += 5;

            ttipTextPtr = ttipTextPtr + sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", doubleSignificantPlaces, ((bidsTab[calculatedIndex] - mouseDnBid) * price_multiplier) * PipsDividerMultiplier);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';
            TOOLTIPLABELCONTENT[ttipTextPtr + 2] = 't';
            TOOLTIPLABELCONTENT[ttipTextPtr + 3] = 'i';
            TOOLTIPLABELCONTENT[ttipTextPtr + 4] = 'c';
            TOOLTIPLABELCONTENT[ttipTextPtr + 5] = 'k';
            TOOLTIPLABELCONTENT[ttipTextPtr + 6] = 's';
            TOOLTIPLABELCONTENT[ttipTextPtr + 7] = ':';
            TOOLTIPLABELCONTENT[ttipTextPtr + 8] = ' ';

            ttipTextPtr += 9;

            _itoa_s(ticksCount, &TOOLTIPLABELCONTENT[ttipTextPtr], 100, 10);

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;

            if (partialToRefresh)
            {
               RefreshTickChartWindow(refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);
            }

            if (mouseDnPosX < posX)
            {
               refreshRect.left = mouseDnPosX - 1;
               refreshRect.right = posX + 1;
            }
            else
            {
               refreshRect.left = posX - 1;
               refreshRect.right = mouseDnPosX + 1;
            }
            refreshRect.top = 1;
            refreshRect.bottom = tick_chart.Height() - 1;

            tick_chart.ApplyAlphaBlend(tChHWnd, refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);

            HDC hDc = GetDC(tChHWnd);
            old_pen = (HPEN)SelectObject(hDc, m_simplePen);

            MoveToEx(hDc, mouseDnPosX, mouseDnPosY, nullptr);
            LineTo(hDc, posX, posY);

            SelectObject(hDc, old_pen);
            ReleaseDC(tChHWnd, hDc);

            partialToRefresh = true;
         }
      }
      else
      {
         if (defTitleChanged)
         {
            SetWindowText(rootWnd, defaultRootTitle);
         }

         defTitleChanged = false;
      }
   }
}
void CTickChartModule::BarChartTTipChanged(int posX, int posY, short LBUTTON_state, wchar_t *defaultRootTitle)
{
   if (seriesIndex > -1)
   {
      static bool partialToRefresh = false;
      static RECT refreshRect = {0, 0, 0, 0};

      static bool defTitleChanged = false;
      int startPoint = (int)tick_chart.GetDataAreaStartPoint();
      if ((uint)(posX - startPoint) < 1000 && (uint)(posY - 1) < 289)
      {
         int ttipTextPtr = 0;

         static int barStartIndex = 0;
         static int mouseDnBarPosX = 0;
         static int mouseDnBarPosY = 0;

         int shift = (posX - startPoint - 999) / (*appSets).barChartCandleWidth;
         int calculatedIndex = (chartSearchIndex / (*appSets).barChartTickSize) + (shift)-1;

         calculatedIndex = std::max(calculatedIndex, 0);

         calculatedIndex *= (*appSets).barChartTickSize;

         if (LBUTTON_state == 0)
         {
            if (partialToRefresh)
            {
               RefreshBarChartWindow(refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);
               partialToRefresh = false;
               refreshRect.bottom = 0;
               refreshRect.left = 0;
               refreshRect.right = 0;
               refreshRect.top = 0;
            }

            mouseDnBarPosX = posX;
            mouseDnBarPosY = posY;
            barStartIndex = calculatedIndex;

            double pointerPrice = ((286 - posY) * bar_chart.GetPointsPerPixel() * symbol_point_size) + bar_chart.VScaleMin();

            time_t t0 = timesTabInMs[calculatedIndex];
            time_t t1 = t0 / 1000;
            uint days = (((t1 * 1158050442) >> 32) + t1 * 49710) >> 32;
            ulong _sc = t1 - (days * 24 * 60 * 60);
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - (mn * 60));

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(t0 - (t1 * 1000));
            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;
         }
         else
         {
            int ind1 = 0;
            int ind2 = 0;
            if (barStartIndex < calculatedIndex)
            {
               ind1 = barStartIndex;
               ind2 = calculatedIndex;
            }
            else
            {
               ind1 = calculatedIndex;
               ind2 = barStartIndex;
            }

            ulong barStartTm = timesTabInMs[ind1];
            ulong barEndTm = timesTabInMs[ind2 + (*appSets).barChartTickSize - 1];
            ulong timeDiff = barEndTm - barStartTm;

            int barsCount = abs(mouseDnBarPosX - posX) / (*appSets).barChartCandleWidth;

            double priceDiff = (abs(mouseDnBarPosY - posY) * bar_chart.GetPointsPerPixel()) * PipsDividerMultiplier;

            TOOLTIPLABELCONTENT[0] = 't';
            TOOLTIPLABELCONTENT[1] = 'm';
            TOOLTIPLABELCONTENT[2] = 'D';
            TOOLTIPLABELCONTENT[3] = 'f';
            TOOLTIPLABELCONTENT[4] = ':';
            TOOLTIPLABELCONTENT[5] = ' ';

            time_t td = timeDiff / 1000;
            uint days = (((td * 1158050442) >> 32) + td * 49710) >> 32;
            ulong _sc = td - (days * 24 * 60 * 60);
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - (mn * 60));

            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[8] = ':';
            TOOLTIPLABELCONTENT[9] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[10] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[11] = ':';
            TOOLTIPLABELCONTENT[12] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[13] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[14] = '.';

            uint mil = (uint)(timeDiff - (td * 1000));
            TOOLTIPLABELCONTENT[15] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[16] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[17] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[18] = ';';
            TOOLTIPLABELCONTENT[19] = ' ';
            TOOLTIPLABELCONTENT[20] = 'p';
            TOOLTIPLABELCONTENT[21] = 'r';
            TOOLTIPLABELCONTENT[22] = 'c';
            TOOLTIPLABELCONTENT[23] = 'D';
            TOOLTIPLABELCONTENT[24] = 'f';
            TOOLTIPLABELCONTENT[25] = ':';
            TOOLTIPLABELCONTENT[26] = ' ';

            ttipTextPtr = 27 + sprintf_s(&TOOLTIPLABELCONTENT[27], 100, "%.*f", doubleSignificantPlaces, priceDiff);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';
            TOOLTIPLABELCONTENT[ttipTextPtr + 2] = 'b';
            TOOLTIPLABELCONTENT[ttipTextPtr + 3] = 'a';
            TOOLTIPLABELCONTENT[ttipTextPtr + 4] = 'r';
            TOOLTIPLABELCONTENT[ttipTextPtr + 5] = 's';
            TOOLTIPLABELCONTENT[ttipTextPtr + 6] = ':';
            TOOLTIPLABELCONTENT[ttipTextPtr + 7] = ' ';

            ttipTextPtr += 8;

            _itoa_s(barsCount, &TOOLTIPLABELCONTENT[ttipTextPtr], 100, 10);

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;

            if (partialToRefresh)
            {
               RefreshBarChartWindow(refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);
            }

            if (mouseDnBarPosX < posX)
            {
               refreshRect.left = mouseDnBarPosX - 1;
               refreshRect.right = posX + 1;
            }
            else
            {
               refreshRect.left = posX - 1;
               refreshRect.right = mouseDnBarPosX + 1;
            }
            refreshRect.top = 1;
            refreshRect.bottom = bar_chart.Height() - 1;

            bar_chart.ApplyAlphaBlend(bChHWnd, refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);

            HDC hDc = GetDC(bChHWnd);
            old_pen = (HPEN)SelectObject(hDc, m_simplePen);

            MoveToEx(hDc, mouseDnBarPosX, mouseDnBarPosY, nullptr);
            LineTo(hDc, posX, posY);

            SelectObject(hDc, old_pen);
            ReleaseDC(bChHWnd, hDc);

            partialToRefresh = true;
         }
      }
      else
      {
         if (defTitleChanged)
         {
            SetWindowText(rootWnd, defaultRootTitle);
         }

         defTitleChanged = false;
      }
   }
}
void CTickChartModule::ReplayBtnClicked()
{
   if ((*appSets).replayModeOn)
   {
      if ((*appSets).realTempo)
      {
         if (((appSets->tickChartZoom) & 1) != 0) // tickSampleWidth==1
         {
            // Step forward == 2, to avoid tick chart flickering
            (*appSets).tickOffsetValue = 2;
         }
         else
         {
            (*appSets).tickOffsetValue = 1;
         }
      }
      StepForwardBtnClicked();
   }
   else
   {
      if ((*appSets).realTempo)
      {
         BOOL success = 0;
         uint tickInterv = (uint)GetDlgItemInt(hWnd, IDC_STEP_SIZE_EDT, &success, 0);
         if (success != 0)
         {
            if (tickInterv > 1)
            {
               (*appSets).tickOffsetValue = tickInterv;
            }
         }
      }
   }
}
void CTickChartModule::SetRangeBtnClicked()
{
   if (chartSearchIndex == -1)
   {
      return;
   }
   BOOL success = 0;
   uint newRange = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, 0);
   if (success != 0)
   {
      if (newRange > 1)
      {
         NormalizationArgs args = {(newRange / 2.0) * symbol_point_size, symbol_digits};
         double halfDistance = NormalizeDouble(args);
         args.value = (bidsTab[chartSearchIndex] + asksTab[chartSearchIndex]) / 2.0;
         double meanPrice = NormalizeDouble(args);
         args.value = meanPrice + halfDistance;
         upRangeLineValue = NormalizeDouble(args);
         args.value = meanPrice - halfDistance;
         downRangeLineValue = NormalizeDouble(args);

         char charBuffer[50];
         sprintf_s(charBuffer, "%.*f", doubleSignificantPlaces, (abs(upRangeLineValue - downRangeLineValue) * price_multiplier) * PipsDividerMultiplier);

         size_t len = strlen(charBuffer);

         charBuffer[len] = ' ';
         charBuffer[len + 1] = 'p';
         charBuffer[len + 2] = 'i';
         charBuffer[len + 3] = 'p';
         charBuffer[len + 4] = 's';
         charBuffer[len + 5] = '\0';

         SetDlgItemTextA(hWnd, IDC_INFOLABEL, charBuffer);

         tick_chart.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tick_chart.VScaleParams(upRangeLineValue, downRangeLineValue, 10, true);

         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }
   }
}
void CTickChartModule::SetBarTickSizeBtnClicked()
{
   BOOL success = 0;
   uint newVal = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, 0);
   if (success != 0)
   {
      if (newVal > 1)
      {
         (*appSets).barChartTickSize = newVal;
         if (chartSearchIndex > -1)
         {
            RecalculateBarChart(chartSearchIndex, false);
            bar_chart.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
            bar_chart.UpdateChart(true);
         }
         else
         {
            bar_chart.SetBarChartTickSize((*appSets).barChartTickSize, false);
         }
      }
   }
}
void CTickChartModule::RecalculateBarChart(int endInd, bool redraw)
{
   barChartTickSizeCounter = 0;
   bar_chart.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0);

   barChartHigh = DBL_MAX * -1.0;
   barChartLow = DBL_MAX;

   int i = 0;

   for (; i < endInd; i++)
   {
      barChartTickSizeCounter++;
      barChartHigh = std::max(barChartHigh, bidsTab[i]);

      barChartLow = std::min(barChartLow, bidsTab[i]);

      if (barChartTickSizeCounter == (*appSets).barChartTickSize)
      {
         ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
         bar_chart.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
         barChartTickSizeCounter = 0;
      }
      bar_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
   }
   bar_chart.SetBarChartTickSize((*appSets).barChartTickSize, redraw);
}
void CTickChartModule::MoveRangeUpBtnClicked()
{
   if (seriesIndex == -1)
   {
      return;
   }
   BOOL success = 0;
   uint valueToMove = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, 0);
   if (success != 0)
   {
      if (valueToMove > 0)
      {
         NormalizationArgs args = {upRangeLineValue + (valueToMove * symbol_point_size), symbol_digits};
         upRangeLineValue = NormalizeDouble(args);
         if ((*appSets).freezeRanges)
         {
            args.value = downRangeLineValue + (valueToMove * symbol_point_size);
            downRangeLineValue = NormalizeDouble(args);
         }

         tick_chart.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tick_chart.VScaleParams(upRangeLineValue, downRangeLineValue, 10);

         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -MOVE_RANGE_UP);
      }
   }
}
void CTickChartModule::MoveRangeDownBtnClicked()
{
   if (seriesIndex == -1)
   {
      return;
   }
   BOOL success = 0;
   uint valueToMove = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, 0);
   if (success != 0)
   {
      if (valueToMove > 0)
      {
         NormalizationArgs args = {upRangeLineValue - (valueToMove * symbol_point_size), symbol_digits};
         upRangeLineValue = NormalizeDouble(args);
         if ((*appSets).freezeRanges)
         {
            args.value = downRangeLineValue - (valueToMove * symbol_point_size);
            downRangeLineValue = NormalizeDouble(args);
         }

         tick_chart.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tick_chart.VScaleParams(upRangeLineValue, downRangeLineValue, 10);

         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -MOVE_RANGE_DOWN);
      }
   }
}
void CTickChartModule::SignedLevelsEditChBxChanged()
{
}
void CTickChartModule::AddSignedLevelBtnClicked()
{
   if (seriesIndex == -1)
   {
      return;
   }
   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -ADD_SIGNED_LEVEL);
}
void CTickChartModule::DeleteSignedLevelsBtnClicked()
{
   if (seriesIndex == -1)
   {
      return;
   }
   if (MessageBox(hWnd, L"Delete all '_TT' HLine objects?", L"Deleting HLine objects", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
   {
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -DELETE_ALL_SIGNED_LEVELS);
   }
}
void CTickChartModule::TickChartZoomChanged()
{
   tick_chart.SetTickSampleWidth((*appSets).tickChartZoom, false);

   if (seriesIndex == -1)
   {
      return;
   }

   tick_chart.UpdateChart();
}
void CTickChartModule::BarChartCandleWidthChanged()
{
   bar_chart.SetBarChartBarWidth((*appSets).barChartCandleWidth, seriesIndex > -1);
}
void CTickChartModule::TickChartTimeSepChanged()
{
   tick_chart.SetInterval((*appSets).tickChartTimeSep, seriesIndex > -1);
}
void CTickChartModule::BarChartTimeSepChanged()
{
   bar_chart.SetInterval((*appSets).barChartTimeSep, seriesIndex > -1);
}
void CTickChartModule::FreezeRanges()
{
}
void CTickChartModule::ResetCharts()
{
   if (MessageBox(hWnd, L"Reset current app state?", L"Data reset", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
   {
      ResetData();
   }
}
void CTickChartModule::RefreshTickChartWindow()
{
   tick_chart.RefreshWindow();
}
void CTickChartModule::RefreshTickChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   tick_chart.RefreshWindow(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CTickChartModule::RefreshBarChartWindow()
{
   bar_chart.RefreshWindow();
}
void CTickChartModule::RefreshBarChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   bar_chart.RefreshWindow(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CTickChartModule::AskLineVisChanged()
{
   tick_chart.ChartVisibility(0, (int)(*appSets).askLineVis, seriesIndex > -1);
}
void CTickChartModule::BidLineVisChanged()
{
   tick_chart.ChartVisibility(1, (int)(*appSets).bidLineVis, seriesIndex > -1);
};
void CTickChartModule::SignedLevelsVisChanged()
{
   if (seriesIndex == -1)
   {
      return;
   }

   if ((*appSets).signedLevelsVis)
   {
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SHOW_SIGNED_LEVELS);
   }

   tick_chart.ShowSignedLevels((*appSets).signedLevelsVis, !(*appSets).signedLevelsVis);
   bar_chart.ShowSignedLevels((*appSets).signedLevelsVis, !(*appSets).signedLevelsVis);
}
void CTickChartModule::UpdateVisLevels(bool updateChart)
{
   tick_chart.ShowSignedLevels((*appSets).signedLevelsVis, updateChart);
   bar_chart.ShowSignedLevels((*appSets).signedLevelsVis, updateChart);
}
void CTickChartModule::AutoMovingRangeChanged()
{
}
void CTickChartModule::MProfileAskVisChanged()
{
   if (symbol_digits <= 5)
   {
      tick_chart.ShowMProfileDataAsk((*appSets).mProfileAskVis, seriesIndex > -1);
      bar_chart.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, seriesIndex > -1);
   }
}
void CTickChartModule::MProfileBidVisChanged()
{
   if (symbol_digits <= 5)
   {
      tick_chart.ShowMProfileDataBid((*appSets).mProfileBidVis, seriesIndex > -1);
      bar_chart.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, seriesIndex > -1);
   }
}
void CTickChartModule::TickChartVisChanged()
{
   tick_chart.ShowMainPlot((*appSets).tickChartVis, seriesIndex > -1);
}
void CTickChartModule::BarChartVisChanged()
{
   bar_chart.ShowMainPlot((*appSets).barChartVis, seriesIndex > -1);
}
void CTickChartModule::TimeParameterVisChanged()
{
   tick_chart.ShowTimeParameter((*appSets).timeParamVis, seriesIndex > -1);
   bar_chart.ShowTimeParameter((*appSets).timeParamVis, seriesIndex > -1);
}
void CTickChartModule::CumulativeAskVisChanged()
{
   tick_chart.ShowCumulativeDataAsk((*appSets).cumulativeAskVis, seriesIndex > -1);
}
void CTickChartModule::CumulativeBidVisChanged()
{
   tick_chart.ShowCumulativeDataBid((*appSets).cumulativeBidVis, seriesIndex > -1);
}
void CTickChartModule::DistanceVisChanged()
{
   if (EnableSpeedStats)
   {
      tick_chart.ShowTravelledDistance((*appSets).distanceVis, seriesIndex > -1);
   }
}
void CTickChartModule::RoadVisChanged()
{
   if (EnableSpeedStats)
   {
      tick_chart.ShowTravelledRoad((*appSets).roadVis, seriesIndex > -1);
   }
}
void CTickChartModule::TicksArrivedVisChanged()
{
   if (EnableSpeedStats)
   {
      tick_chart.ShowTicksArrived((*appSets).ticksArrivedVis, seriesIndex > -1);
   }
}
void CTickChartModule::EventsOnTickChartVisChanged()
{
}
void CTickChartModule::EventsOnBarChartVisChanged()
{
}
void CTickChartModule::OrdersVisChanged()
{
   if ((*appSets).ordersVis)
   {
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SHOW_ORDERS);
   }
   else
   {
      tick_chart.ShowOrderPoint(false, seriesIndex > -1);
      bar_chart.ShowOrderPoint(false, seriesIndex > -1);
   }
}
void CTickChartModule::ColorTimeParamChanged()
{
   tick_chart.ColorTimeParameter((*appSets).colorTimeParam, seriesIndex > -1);
   bar_chart.ColorTimeParameter((*appSets).colorTimeParam, seriesIndex > -1);
}
void CTickChartModule::ZoomTimeParamChanged()
{
   UpdateBiggerBarsData(seriesIndex > -1);
}

bool CTickChartModule::SaveTicksClicked(LPCTSTR pszFileName)
{
   HANDLE hFile = nullptr;
   bool bSuccess = false;

   hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize = 0;

      std::string strContent;
      char temp[1000];

      for (int i = 0; i < seriesIndex; i++)
      {
         _i64toa_s(timesTabInMs[i] / 1000, temp, 100, 10);
         strContent.append(temp);
         strContent += ',';
         _itoa_s(timesTabInMs[i] % 1000, temp, 100, 10);
         strContent.append(temp);
         strContent += ',';

         sprintf_s(temp, 100, "%.*f", symbol_digits, asksTab[i]);
         strContent.append(temp);
         strContent += ',';
         sprintf_s(temp, 100, "%.*f", symbol_digits, bidsTab[i]);
         strContent.append(temp);
         strContent += '\n';
      }
      dwFileSize = (DWORD)strContent.size();
      DWORD dwWritten = 0;

      if (WriteFile(hFile, strContent.c_str(), dwFileSize, &dwWritten, nullptr) != 0)
      {
         bSuccess = true;
         SetDlgItemTextA(hWnd, IDC_MAIN_EDT, "File Saved");
      }

      CloseHandle(hFile);
   }
   return (bSuccess);
}
int CTickChartModule::GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
   UINT num = 0;
   UINT size = 0;
   Gdiplus::GetImageEncodersSize(&num, &size);
   if (size == 0)
   {
      return -1;
   }

   auto *pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
   if (pImageCodecInfo == nullptr)
   {
      return -1;
   }

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for (UINT j = 0; j < num; ++j)
   {
      if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;
      }
   }
   free(pImageCodecInfo);
   return -1;
}
bool CTickChartModule::PrintScreenClicked(LPCTSTR pszFileName)
{
   HDC hDCScreen = ::GetDC(rootWnd);
   HDC hDCMemory = ::CreateCompatibleDC(hDCScreen);

   int nWidthScreen = 1100;
   int nHeightScreen = 726;

   HBITMAP hBitmap = ::CreateCompatibleBitmap(hDCScreen, nWidthScreen, nHeightScreen);
   HGDIOBJ hOldBitmap = ::SelectObject(hDCMemory, hBitmap);

   ::BitBlt(hDCMemory, 0, 0, nWidthScreen, nHeightScreen, hDCScreen, 0, 0, SRCCOPY);

   bool success = false;
   {
      // GDI+ Bitmap will correctly release the handle at the end of this block (RAII)
      Gdiplus::Bitmap bitmap(hBitmap, nullptr);
      CLSID pngClsid;
      if (GetEncoderClsid(L"image/png", &pngClsid) != -1)
      {
         Gdiplus::Status status = bitmap.Save(pszFileName, &pngClsid, nullptr);
         success = (status == Gdiplus::Ok);
      }
   }

   // Cleaning up GDI resources (must happen AFTER destroying the Gdiplus::Bitmap object)
   ::SelectObject(hDCMemory, hOldBitmap);
   ::DeleteObject(hBitmap);
   ::DeleteDC(hDCMemory);
   ::ReleaseDC(rootWnd, hDCScreen);

   return success;
}
void CTickChartModule::RealTempoChanged()
{
}
void CTickChartModule::TimerIntervalChanged()
{
   if ((*appSets).replayModeOn)
   {
      // EventSetMillisecondTimer((*appSets).timerInterval);
   }
}
bool CTickChartModule::ReadTickDataFromFile(LPCTSTR pszFileName)
{
   HANDLE hFile = nullptr;
   bool bSuccess = false;

   hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize = 0;

      dwFileSize = GetFileSize(hFile, nullptr);
      if (dwFileSize != 0xFFFFFFFF)
      {
         char *pszFileText = nullptr;
         pszFileText = new char[dwFileSize];

         if (pszFileText != nullptr)
         {
            DWORD dwRead = 0;

            if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, nullptr) != 0)
            {
               auto *lngPtr = (long long *)pszFileText;
               auto *dblPtr = (double *)pszFileText;

               dataSize = (int)lngPtr[0];

               if (dataSize > 0)
               {
                  delete[] timesTabInMs;
                  delete[] bidsTab;
                  delete[] asksTab;
                  delete[] realTempoValsTab;

                  timesTabInMs = new __int64[dataSize];
                  bidsTab = new double[dataSize];
                  asksTab = new double[dataSize];
                  realTempoValsTab = new int[dataSize];

                  realTempoValsTab[0] = (*appSets).timerInterval;

                  int dataIterator = 0;
                  for (int i = 1; i < dataSize * 4; i += 4, dataIterator++)
                  {
                     timesTabInMs[dataIterator] = lngPtr[i] * 1000 + lngPtr[i + 1];
                     asksTab[dataIterator] = dblPtr[i + 2];
                     bidsTab[dataIterator] = dblPtr[i + 3];
                     if (dataIterator > 0)
                     {
                        realTempoValsTab[dataIterator] = (int)(timesTabInMs[dataIterator] - timesTabInMs[dataIterator - 1]);
                     }
                  }
                  SetDlgItemInt(hWnd, IDC_INFOLABEL, (UINT)dataIterator, 0);
                  SetDlgItemText(hWnd, IDC_DATELABEL, L"0000.00.00");
                  TimeSepVLine = timesTabInMs[0];
               }
            }
         }
         delete[] pszFileText;
      }
      CloseHandle(hFile);
   }
   return (bSuccess);
}
void CTickChartModule::ResetData()
{
   // seriesIndex = -1;
   // barChartHigh = DBL_MAX * -1.0;
   // barChartLow = DBL_MAX;

   // barChartTickSizeCounter = 0;
   // chartSearchIndex = -1;

   // tick_chart.FillSeries(last_tick.ask, last_tick.bid, 0, 0, 0, 0);
   // bar_chart.FillSeries(last_tick.bid, last_tick.bid, last_tick.bid, last_tick.bid, 0, 0);
}
void CTickChartModule::UpdateBiggerBarsData(bool updateChart)
{
   uint newMultiplier = 1;
   if ((*appSets).zoomTimeParam)
   {
      newMultiplier = 4;
   }

   tick_chart.BiggerBarsData(newMultiplier, updateChart);
   bar_chart.BiggerBarsData(newMultiplier, updateChart);
}
int CTickChartModule::OnTimer()
{
   int nextInterval = 0;

   if ((*appSets).realTempo)
   {
      if (chartSearchIndex > -1)
      {
         LARGE_INTEGER StartingTime;
         LARGE_INTEGER EndingTime;
         LARGE_INTEGER ElapsedMicroseconds;
         LARGE_INTEGER Frequency;
         QueryPerformanceFrequency(&Frequency);
         QueryPerformanceCounter(&StartingTime);

         StepForwardBtnClicked();

         QueryPerformanceCounter(&EndingTime);
         ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
         auto interval = static_cast<long long>(ElapsedMicroseconds.QuadPart * 1000 / (Frequency.QuadPart));

         nextInterval = realTempoValsTab[chartSearchIndex] - (int)(interval);
         if (((appSets->tickChartZoom) & 1) != 0) // tickSampleWidth==1 Step forward == 2, to avoid tick chart flickering
         {
            if (chartSearchIndex < seriesIndex)
            {
               nextInterval += realTempoValsTab[chartSearchIndex + 1];
            }
         }
         if (nextInterval <= 0)
         {
            nextInterval = 1;
         }
      }
   }
   else
   {
      StepForwardBtnClicked();
   }
   return (nextInterval);
}
void CTickChartModule::AppendLevels(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update)
{
   tick_chart.AppendSignedLevels(levels, levelsSize, descriptions, descrSize, false);
   bar_chart.AppendSignedLevels(levels, levelsSize, descriptions, descrSize, false);
   if (update && seriesIndex > -1)
   {
      PostMessageA(rootWnd, UPDATE_CHARTS, 0, 1);
   }
}
void CTickChartModule::AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size, const bool update)
{
   tick_chart.AppendTransactionsPoints(transactions, descriptions, size);
   bar_chart.AppendTransactionsPoints(transactions, descriptions, size);
   if (update && seriesIndex > -1)
   {
      PostMessageA(rootWnd, UPDATE_CHARTS, 0, 1);
   }
}
bool CTickChartModule::GetSignedLevelData(double &levelValue, char *levelText, int arrSize)
{
   if (chartSearchIndex > -1)
   {
      levelValue = bidsTab[chartSearchIndex];
      GetDlgItemTextA(hWnd, IDC_MAIN_EDT, levelText, arrSize);
   }
   return (true);
}
bool CTickChartModule::GetTimeAndPriceData(double &priceValue, time_t &timeValue)
{
   if (chartSearchIndex > -1)
   {
      priceValue = bidsTab[chartSearchIndex];
      timeValue = timesTabInMs[chartSearchIndex] / 1000;
   }
   return (true);
}
bool CTickChartModule::GetRangeValData(double &upRangeVal, double &downRangeVal) const
{
   upRangeVal = upRangeLineValue;
   downRangeVal = downRangeLineValue;
   return (true);
}
bool CTickChartModule::RangeLineDragged(double &newValue, int index)
{
   if (seriesIndex == -1)
   {
      return (true);
   }

   if (index == 0)
   {
      if ((*appSets).freezeRanges)
      {
         NormalizationArgs args = {newValue - upRangeLineValue, symbol_digits};
         double difference = NormalizeDouble(args);
         downRangeLineValue = downRangeLineValue + difference;
      }
      upRangeLineValue = newValue;
   }
   else
   {
      if ((*appSets).freezeRanges)
      {
         NormalizationArgs args = {newValue - downRangeLineValue, symbol_digits};
         double difference = NormalizeDouble(args);
         upRangeLineValue = upRangeLineValue + difference;
      }
      downRangeLineValue = newValue;
   }
   tick_chart.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
   tick_chart.VScaleParams(upRangeLineValue, downRangeLineValue, 10, false);

   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -MOVE_RANGE_UP);
   PostMessageA(rootWnd, UPDATE_CHARTS, 1, 1);
   return (true);
}
bool CTickChartModule::TimeSepVLineDragged(__int64 &newValue)
{
   if (seriesIndex == -1)
   {
      return (true);
   }

   if (newValue >= timesTabInMs[0] && newValue <= timesTabInMs[seriesIndex])
   {
      TimeSepVLine = newValue;
      PostMessageA(rootWnd, REWIND_CHARTS, 0, 1);
   }
   return (true);
}
bool CTickChartModule::SetSymbolParameters(SYMBOLSETS &sets, char *date, int dateChars)
{
   PipsDivider = sets.pipsDivider;
   PipsDividerMultiplier = 1.0 / PipsDivider;
   DarkMode = sets.darkMode;
   symbol_point_size = sets.pointValue;
   symbol_digits = sets.digitsCount;
   price_multiplier = POWER_OF_10[symbol_digits];
   ExpandDateRange = sets.expandDateRange;
   ExcludePremarketData = sets.excludePremarketData;
   EnableSpeedStats = sets.enableSpeedStats;

   memcpy(TOOLTIPLABELCONTENT, date, dateChars);

   last_tick.ask = sets.initAsk;
   last_tick.bid = sets.initBid;
   last_tick.time = sets.initTime;

   tick_chart.SetColorMode(DarkMode);
   bar_chart.SetColorMode(DarkMode);

   return (true);
}
bool CTickChartModule::TickDataLoaded(MqlTick tckArray[], int arrSize, char *date, int dateChars)
{
   seriesIndex = -1;
   chartSearchIndex = -1;
   int _dstInd = 0;
   int _srcInd = 0;

   long long _lastTimeInMs = (*appSets).timerInterval;

   bool _excludePremarketData = ExcludePremarketData;
   for (; (_dstInd < dataSize - 1) && (_srcInd < arrSize); _dstInd++, _srcInd++)
   {
      if (_excludePremarketData)
      {
         if (((tckArray[_srcInd].time / 3600) % 24) == 0) // from 00:00:00 to 00:59:59
         {
            _dstInd--;
            continue;
         }
      }

      timesTabInMs[_dstInd] = tckArray[_srcInd].time_msc;
      bidsTab[_dstInd] = tckArray[_srcInd].bid;
      asksTab[_dstInd] = tckArray[_srcInd].ask;
      realTempoValsTab[_dstInd] = (int)(timesTabInMs[_dstInd] - _lastTimeInMs);
      _lastTimeInMs = tckArray[_srcInd].time_msc;
   }
   seriesIndex = _dstInd - 1;

   SetDlgItemTextA(hWnd, IDC_DATELABEL, date);

   if (seriesIndex > 0)
   {
      PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 1);
   }

   return (true);
}
bool CTickChartModule::TickDataPartialLoaded(MqlTick tckArray[], int arrSize)
{
   chartSearchIndex = -1;

   for (int i = seriesIndex; i >= 0; i--)
   {
      if (timesTabInMs[i] == tckArray[0].time_msc)
      {
         seriesIndex = i;
         timesTabInMs[seriesIndex] = tckArray[0].time_msc;
         bidsTab[seriesIndex] = tckArray[0].bid;
         asksTab[seriesIndex] = tckArray[0].ask;
         if (seriesIndex > 0)
         {
            realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
         }
         else
         {
            realTempoValsTab[0] = (*appSets).timerInterval;
         }

         for (int ind = 1; ind < arrSize; ind++)
         {
            seriesIndex++;
            timesTabInMs[seriesIndex] = tckArray[ind].time_msc;
            bidsTab[seriesIndex] = tckArray[ind].bid;
            asksTab[seriesIndex] = tckArray[ind].ask;
            realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
         }
         break;
      }
   }
   if (seriesIndex > 0)
   {
      PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 1);
   }
   return (true);
}
bool CTickChartModule::OnNewTick(MqlTick &lastTick)
{
   if (readyToUse)
   {
      if (seriesIndex < dataSize - 1)
      {
         seriesIndex++;
      }
      // Critical error: throws assembly code in the MT5 terminal
      timesTabInMs[seriesIndex] = lastTick.time_msc;
      bidsTab[seriesIndex] = lastTick.bid;
      asksTab[seriesIndex] = lastTick.ask;
      if (seriesIndex > 0)
      {
         realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
      }
      else
      {
         realTempoValsTab[0] = (*appSets).timerInterval;
      }

      if ((*appSets).chartAutoScroll)
      {
         if (((appSets->tickChartZoom) & 1) != 0) // tickSampleWidth==1
         {
            if ((seriesIndex & 1) != 0)
            { // seriesIndex&1 - when index is odd (update every 2 ticks)
               PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 0);
            }
            return (true);
         }
         PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 0);
      }
   }
   return (true);
}
bool CTickChartModule::NewRatesLoaded(MqlTick tckArray[], int arrSize)
{
   if (readyToUse)
   {
      if (arrSize == 1)
      {
         if (seriesIndex < dataSize - 1)
         {
            seriesIndex++;
         }
         timesTabInMs[seriesIndex] = tckArray[0].time_msc;
         bidsTab[seriesIndex] = tckArray[0].bid;
         asksTab[seriesIndex] = tckArray[0].ask;
         if (seriesIndex > 0)
         {
            realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
         }
         else
         {
            realTempoValsTab[0] = (*appSets).timerInterval;
         }
      }
      else
      {
         int stopIdx = (seriesIndex + arrSize - 1) < dataSize ? arrSize : dataSize - seriesIndex;
         for (int ind = 0; ind < arrSize; ind++)
         {
            seriesIndex++;
            timesTabInMs[seriesIndex] = tckArray[ind].time_msc;
            bidsTab[seriesIndex] = tckArray[ind].bid;
            asksTab[seriesIndex] = tckArray[ind].ask;
            if (seriesIndex > 0)
            {
               realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
            }
            else
            {
               realTempoValsTab[0] = (*appSets).timerInterval;
            }
         }
      }

      if ((*appSets).chartAutoScroll)
      {
         PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 0);
      }
   }
   return (true);
}
void CTickChartModule::AutoScrollUpdate(bool forceVScaleUpdate)
{
   if (chartSearchIndex == -1)
   {
      barChartTickSizeCounter = 0;
      barChartHigh = DBL_MAX * -1.0;
      barChartLow = DBL_MAX;

      tick_chart.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
      bar_chart.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);
   }

   bool updateBarChart = false;

   int bidChange = 0;
   int askChange = 0;
   double wholeRoad = 0;
   short priceDistance = 0;
   short ticksElapsed = 0;
   ulong parameter = 0;

   for (int i = chartSearchIndex + 1; i <= seriesIndex; i++)
   {
      chartSearchIndex++;
      barChartTickSizeCounter++;

      bidChange = 0;
      askChange = 0;
      wholeRoad = 0;
      priceDistance = 0;
      ticksElapsed = 0;
      if (i > 0)
      {
         parameter = (ulong)(timesTabInMs[i] - timesTabInMs[i - 1]);

         if (bidsTab[i] > bidsTab[i - 1])
         {
            bidChange = 1;
         }
         else if (bidsTab[i] < bidsTab[i - 1])
         {
            bidChange = -1;
         }

         if (asksTab[i] > asksTab[i - 1])
         {
            askChange = 1;
         }
         else if (asksTab[i] < asksTab[i - 1])
         {
            askChange = -1;
         }

         if (EnableSpeedStats)
         {
            for (int j = i - 1; j > 0; j--)
            {
               wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]) * price_multiplier;
               if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
               {
                  NormalizationArgs args = {(bidsTab[i - 1] - bidsTab[j]) * price_multiplier, 0};
                  priceDistance = (short)NormalizeDouble(args);
                  ticksElapsed = short(i - 1 - j);
                  break;
               }
            }
         }
      }
      tick_chart.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)wholeRoad, priceDistance, ticksElapsed);

      barChartHigh = std::max(barChartHigh, bidsTab[i]);

      barChartLow = std::min(barChartLow, bidsTab[i]);

      if (barChartTickSizeCounter == (*appSets).barChartTickSize)
      {
         parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
         bar_chart.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
         barChartTickSizeCounter = 0;
         updateBarChart = true;
      }

      tick_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      bar_chart.UpdateMarketProfile(asksTab[i], bidsTab[i]);
   }

   if ((*appSets).autoMovingRange)
   {
      double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
      (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
      double meanPrice = (bidsTab[chartSearchIndex] + asksTab[chartSearchIndex]) / 2.0;

      NormalizationArgs args = {meanPrice + halfDistance, symbol_digits};
      double newHPrice = NormalizeDouble(args);
      args.value = meanPrice - halfDistance;
      double newLPrice = NormalizeDouble(args);

      upRangeLineValue = newHPrice;
      downRangeLineValue = newLPrice;
      tick_chart.VScaleParams(newHPrice, newLPrice, 10, false);
      tick_chart.MoveMarketProfileRange(newLPrice, newHPrice);
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
   }
   tick_chart.UpdateChart(forceVScaleUpdate);

   bar_chart.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
   if (updateBarChart)
   {
      bar_chart.UpdateChart(forceVScaleUpdate);
   }
   else
   {
      bar_chart.UpdateCurrentPriceLevel();
   }
}
bool CTickChartModule::UpdateCharts(byte mode, bool forceVScaleUpdate)
{
   switch (mode)
   {
   case 0:
   {
      tick_chart.UpdateChart(forceVScaleUpdate);
      bar_chart.UpdateChart(forceVScaleUpdate);
   }
   break;
   case 1:
      tick_chart.UpdateChart(forceVScaleUpdate);
      break;
   // mode==2
   default:
      bar_chart.UpdateChart(forceVScaleUpdate);
      break;
   }
   return (true);
}