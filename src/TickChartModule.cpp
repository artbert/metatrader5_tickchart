#include "StdAfx.h"
#include "TickChartModule.h"

CTickChartModule::CTickChartModule(void)
{
   rootWnd = NULL;
   tChHWnd = NULL;
   bChHWnd = NULL;
   TerminalParentChartHWnd = NULL;
   readyToUse = false;

   timesTabInMs = NULL;
   bidsTab = NULL;
   asksTab = NULL;
   realTempoValsTab = NULL;
   dataSize = 0;

   isCalendarDataRead = false;
   seriesIndex = -1;
   chartSearchIndex = -1;

   upRangeLineValue = 0;
   downRangeLineValue = 0;

   barChartTickSizeCounter = 0;

   milisecondTimerInterval = 1000;
   isTimerOn = false;
   noOfSecondsForCalc = 7;

   PipsDivider = 10;
   PipsDividerMultiplier = 0.1;
   DarkMode = true;
   _Point = 0.00001;
   _DigitsMultiplier = POWER_OF_10[5];
   _Digits = 5;

   ExpandDateRange = true;
   ExcludePremarketData = true;
   EnableSpeedStats = false;

   TimeSepVLine = 0;

   m_gdiplusToken = 0;
}

CTickChartModule::~CTickChartModule(void)
{
   if (timesTabInMs != NULL)
      delete[] timesTabInMs;
   if (bidsTab != NULL)
      delete[] bidsTab;
   if (asksTab != NULL)
      delete[] asksTab;
   if (realTempoValsTab != NULL)
      delete[] realTempoValsTab;

   DeleteObject(m_simplePen);

   tickChart1.Destroy();
   barChart1.Destroy();
}

bool CTickChartModule::Initialize(HWND terminalParent, HWND rootWindow, HWND tickChartHWnd, HWND barChartHWnd, HWND toolBoxHWnd)
{
   dataSize = 1000000;

   if (timesTabInMs != NULL)
      delete[] timesTabInMs;
   if (bidsTab != NULL)
      delete[] bidsTab;
   if (asksTab != NULL)
      delete[] asksTab;
   if (realTempoValsTab != NULL)
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
   double midPrice = NormalizeDouble((last_tick.bid + last_tick.ask) / 2.0, _Digits);
   upRangeLineValue = NormalizeDouble(midPrice + 100 * _Point, _Digits);
   downRangeLineValue = NormalizeDouble(midPrice - 100 * _Point, _Digits);

   TerminalParentChartHWnd = terminalParent;
   rootWnd = rootWindow;
   tChHWnd = tickChartHWnd;
   bChHWnd = barChartHWnd;
   hWnd = toolBoxHWnd;

   PipsDivider = 10;
   PipsDividerMultiplier = 0.1;
   tickChart1.Create(tickChartHWnd, 1100, 428, _Point, _Digits);
   barChart1.Create(barChartHWnd, 1100, 300, _Point, _Digits);

   tickChart1.ShowScaleTop(false, false);
   tickChart1.ShowScaleRight(false, false);
   tickChart1.ShowScaleLeft(true, false);
   tickChart1.ShowScaleBottom(false, false);
   tickChart1.ShowLegend(false, false);
   tickChart1.ShowGrid(true, false);
   tickChart1.SetScaleDigits(_Digits);
   tickChart1.ShowDescriptors(false, false);
   tickChart1.SetPipsDivider((int)PipsDivider);
   doubleSignificantPlaces = (int)log10((double)PipsDivider);

   char charBuffer[50];
   sprintf_s(charBuffer, "%.*f", doubleSignificantPlaces, (abs(upRangeLineValue - downRangeLineValue) * _DigitsMultiplier) * PipsDividerMultiplier);

   size_t len = strlen(charBuffer);

   charBuffer[len] = ' ';
   charBuffer[len + 1] = 'p';
   charBuffer[len + 2] = 'i';
   charBuffer[len + 3] = 'p';
   charBuffer[len + 4] = 's';
   charBuffer[len + 5] = '\0';

   SetDlgItemTextA(hWnd, IDC_INFOLABEL, charBuffer);

   tickChart1.VScaleParams(upRangeLineValue, downRangeLineValue, 10, false);
   tickChart1.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);

   if (DarkMode)
   {
      tickChart1.ColorBackground(XRGB_gdi(0x00, 0x00, 0x00), false);
      tickChart1.ColorBorder(XRGB_gdi(0xFF, 0xFF, 0xFF) - tickChart1.ColorBorder(), false);
      tickChart1.ColorText(XRGB_gdi(0xFF, 0xFF, 0xFF) - tickChart1.ColorText(), false);
      tickChart1.ColorGrid(XRGB_gdi(0xFF, 0xFF, 0xFF) - tickChart1.ColorGrid(), false);
      tickChart1.TextColorSet(tickChart1.ColorText());
   }

   barChart1.ShowScaleTop(false, false);
   barChart1.ShowScaleRight(false, false);
   barChart1.ShowScaleLeft(true, false);
   barChart1.ShowScaleBottom(false, false);
   barChart1.ShowLegend(false, false);
   barChart1.ShowGrid(true, false);
   barChart1.SetScaleDigits(_Digits);
   barChart1.ShowDescriptors(false, false);
   barChart1.SetPipsDivider((int)PipsDivider);

   if (DarkMode)
   {
      barChart1.ColorBackground(XRGB_gdi(0x00, 0x00, 0x00), false);
      barChart1.ColorBorder(XRGB_gdi(0xFF, 0xFF, 0xFF) - barChart1.ColorBorder(), false);
      barChart1.ColorText(XRGB_gdi(0xFF, 0xFF, 0xFF) - barChart1.ColorText(), false);
      barChart1.ColorGrid(XRGB_gdi(0xFF, 0xFF, 0xFF) - barChart1.ColorGrid(), false);
   }

   tickChart1.CreatePixelFontSet(tickChart1.ColorBackground());
   barChart1.CreatePixelFontSet(barChart1.ColorBackground());

   m_simplePen = CreatePen(0, 1, tickChart1.ColorText());

   tickChart1.ChartVisibility(0, (int)(*appSets).askLineVis, false);
   tickChart1.ChartVisibility(1, (int)(*appSets).bidLineVis, false);
   tickChart1.ShowCumulativeDataBid((*appSets).cumulativeBidVis, false);
   tickChart1.ShowCumulativeDataAsk((*appSets).cumulativeAskVis, false);

   if (_Digits <= 5)
   {
      tickChart1.ShowMProfileDataAsk((*appSets).mProfileAskVis, false);
      tickChart1.ShowMProfileDataBid((*appSets).mProfileBidVis, false);
      barChart1.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, false);
   }

   tickChart1.SetInterval((*appSets).tickChartTimeSep, false);
   barChart1.SetInterval((*appSets).barChartTimeSep, false);

   barChart1.SetBarChartBarWidth((*appSets).barChartCandleWidth, false);

   tickChart1.ShowTimeParameter((*appSets).timeParamVis, false);
   barChart1.ShowTimeParameter((*appSets).timeParamVis, false);
   tickChart1.ShowMainPlot((*appSets).tickChartVis, false);
   barChart1.ShowMainPlot((*appSets).barChartVis, false);

   tickChart1.ShowSignedLevels((*appSets).signedLevelsVis, false);
   barChart1.ShowSignedLevels((*appSets).signedLevelsVis, false);
   tickChart1.SetTickSampleWidth((*appSets).tickChartZoom, false);
   tickChart1.ColorTimeParameter((*appSets).colorTimeParam, false);
   barChart1.ColorTimeParameter((*appSets).colorTimeParam, false);
   tickChart1.ShowCalendarEvents((*appSets).eventsOnTickChartVis, false);
   barChart1.ShowCalendarEvents((*appSets).eventsOnBarChartVis, false);
   barChart1.SetBarChartTickSize((*appSets).barChartTickSize, false);

   tickChart1.FillSeries(last_tick.ask, last_tick.bid, 0, 0, 0, 0);
   barChart1.FillSeries(last_tick.bid, last_tick.bid, last_tick.bid, last_tick.bid, 0, 0, false);

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
   if (timesTabInMs != NULL)
   {
      delete[] timesTabInMs;
      timesTabInMs = NULL;
   }
   if (bidsTab != NULL)
   {
      delete[] bidsTab;
      bidsTab = NULL;
   }
   if (asksTab != NULL)
   {
      delete[] asksTab;
      asksTab = NULL;
   }
   if (realTempoValsTab != NULL)
   {
      delete[] realTempoValsTab;
      realTempoValsTab = NULL;
   }

   rootWnd = NULL;
   tChHWnd = NULL;
   bChHWnd = NULL;

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
   _Point = 0.00001;
   _Digits = 5;

   ExpandDateRange = true;
   ExcludePremarketData = true;
   EnableSpeedStats = true;

   TimeSepVLine = 0;

   tickChart1.Destroy();
   barChart1.Destroy();
   DeleteObject(m_simplePen);
   m_simplePen = NULL;

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
   BOOL success;
   uint minutes = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, false);
   if (success)
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
      return;

   if (!(*appSets).chartAutoScroll)
   {
      if (seriesIndex == chartSearchIndex)
         return;

      if (chartSearchIndex == -1)
      {
         tickChart1.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
         barChart1.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);
         barChartTickSizeCounter = 0;
         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
      }
      int noOfTicks = (*appSets).tickOffsetValue;

      int tickCounter = 0;
      int i = chartSearchIndex + 1;
      int lastEndIndex = -1;
      if (chartSearchIndex > -1)
         lastEndIndex = chartSearchIndex;

      bool updateBarChart = false;
      uint parameter = 0;
      for (; i <= seriesIndex; i++)
      {
         if (tickCounter >= noOfTicks)
            break;

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
               bidChange = 1;
            else if (bidsTab[i] < bidsTab[i - 1])
               bidChange = -1;

            if (asksTab[i] > asksTab[i - 1])
               askChange = 1;
            else if (asksTab[i] < asksTab[i - 1])
               askChange = -1;

            if (EnableSpeedStats)
            {
               for (int j = i - 1; j > 0; j--)
               {
                  wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]) * _DigitsMultiplier;
                  if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
                  {
                     priceDistance = (short)NormalizeDouble((bidsTab[i - 1] - bidsTab[j]) * _DigitsMultiplier, 0);
                     ticksElapsed = short(i - 1 - j);
                     break;
                  }
               }
            }
         }
         tickChart1.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)wholeRoad, priceDistance, ticksElapsed);

         if (barChartHigh < bidsTab[i])
            barChartHigh = bidsTab[i];

         if (barChartLow > bidsTab[i])
            barChartLow = bidsTab[i];

         if (barChartTickSizeCounter == (*appSets).barChartTickSize)
         {
            ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];

            barChart1.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

            barChartHigh = DBL_MAX * -1.0;
            barChartLow = DBL_MAX;
            barChartTickSizeCounter = 0;
            updateBarChart = true;
         }

         tickCounter++;

         tickChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
         barChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      }

      if ((*appSets).autoMovingRange && i > 0)
      {
         double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
         // instead of abs() for double
         (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
         double meanPrice = (bidsTab[i - 1] + asksTab[i - 1]) / 2.0;
         double newHPrice = NormalizeDouble(meanPrice + halfDistance, _Digits);
         double newLPrice = NormalizeDouble(meanPrice - halfDistance, _Digits);
         upRangeLineValue = newHPrice;
         downRangeLineValue = newLPrice;
         tickChart1.VScaleParams(newHPrice, newLPrice, 10, false);
         tickChart1.MoveMarketProfileRange(newLPrice, newHPrice);
         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }

      tickChart1.UpdateChart();

      barChart1.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
      if (updateBarChart)
         barChart1.UpdateChart();
      else
         barChart1.UpdateCurrentPriceLevel();

      time_t _time = timesTabInMs[chartSearchIndex] / 1000;
      tm timeinfo = {0};
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
      return;

   if (!(*appSets).chartAutoScroll)
   {
      int upLimit = 0;
      if (rewind)
      {
         for (int n = 1; n < seriesIndex; n++)
         {
            if ((timesTabInMs[n] >= TimeSepVLine) && (timesTabInMs[n - 1] <= TimeSepVLine))
               upLimit = n;
         }
      }
      else
      {
         upLimit = chartSearchIndex - (*appSets).tickOffsetValue + 1;
      }

      if (upLimit <= 1)
         upLimit = 1;

      tickChart1.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
      barChart1.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);

      double price1 = upRangeLineValue;
      double price2 = downRangeLineValue;
      tickChart1.VScaleParams(price1, price2, 10, false);
      tickChart1.MoveMarketProfileRange(price2, price1);

      barChartHigh = DBL_MAX * -1.0;
      barChartLow = DBL_MAX;
      barChartTickSizeCounter = 0;
      chartSearchIndex = -1;

      int lastEndIndex = -1;
      if (chartSearchIndex > -1)
         lastEndIndex = chartSearchIndex;

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
               bidChange = 1;
            else if (bidsTab[i] < bidsTab[i - 1])
               bidChange = -1;

            if (asksTab[i] > asksTab[i - 1])
               askChange = 1;
            else if (asksTab[i] < asksTab[i - 1])
               askChange = -1;

            if (EnableSpeedStats)
            {
               for (int j = i - 1; j > 0; j--)
               {
                  wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]);
                  if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
                  {
                     priceDistance = (short)NormalizeDouble((bidsTab[i - 1] - bidsTab[j]) * _DigitsMultiplier, 0);
                     ticksElapsed = short(i - 1 - j);
                     break;
                  }
               }
            }
         }
         tickChart1.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)(wholeRoad * _DigitsMultiplier), priceDistance, ticksElapsed);

         if (barChartHigh < bidsTab[i])
            barChartHigh = bidsTab[i];

         if (barChartLow > bidsTab[i])
            barChartLow = bidsTab[i];

         if (barChartTickSizeCounter == (*appSets).barChartTickSize)
         {
            ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
            barChart1.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

            barChartHigh = DBL_MAX * -1.0;
            barChartLow = DBL_MAX;
            barChartTickSizeCounter = 0;
         }

         tickChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
         barChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      }

      if ((*appSets).autoMovingRange && i > 0)
      {

         double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
         (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
         double meanPrice = (bidsTab[i - 1] + asksTab[i - 1]) / 2.0;
         double newHPrice = NormalizeDouble(meanPrice + halfDistance, _Digits);
         double newLPrice = NormalizeDouble(meanPrice - halfDistance, _Digits);
         upRangeLineValue = newHPrice;
         downRangeLineValue = newLPrice;
         tickChart1.VScaleParams(newHPrice, newLPrice, 10, false);
         tickChart1.MoveMarketProfileRange(newLPrice, newHPrice);
         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }

      tickChart1.UpdateChart(forceVScaleUpdate);

      barChart1.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
      barChart1.UpdateChart(forceVScaleUpdate);

      time_t _time = timesTabInMs[chartSearchIndex] / 1000;
      tm timeinfo = {0};
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
      tickChart1.PrepareAlphaBlend(tChHWnd);
      barChart1.PrepareAlphaBlend(bChHWnd);
   }
   else
   {
      tickChart1.ReleaseAlphaBlend();
      barChart1.ReleaseAlphaBlend();
   }
}
void CTickChartModule::TickChartTTipChanged(int posX, int posY, short LBUTTON_state, wchar_t *defaultRootTitle)
{
   if (seriesIndex > -1)
   {
      static bool defTitleChanged = false;
      static bool partialToRefresh = false;
      static RECT refreshRect = {0, 0, 0, 0};
      int startPoint = (int)tickChart1.GetDataAreaStartPoint();

      if ((uint)(posX - startPoint) < 1000 && (uint)(posY - 1) < 426)
      {
         bool insideArea = false;
         int ttipTextPtr = 0;

         static time_t mouseDnTime = 0;
         static double mouseDnAsk = 0;
         static double mouseDnBid = 0;
         static int mouseDnPosX = 0;
         static int mouseDnPosY = 0;

         int shift = 0;

         if ((*appSets).tickChartZoom == 1)
            shift = (posX - startPoint - 1000) * 2;
         else if ((*appSets).tickChartZoom == 2)
            shift = (posX - startPoint - 1000);
         else
            shift = (posX - startPoint - 1000) / 2;

         int calculatedIndex = chartSearchIndex + (shift);

         if (calculatedIndex < 0)
            calculatedIndex = 0;

         if (!LBUTTON_state)
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
            ulong _sc = t1 - days * 24 * 60 * 60;
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - mn * 60);

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(t0 - t1 * 1000);

            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';

            ttipTextPtr = 14 + sprintf_s(&TOOLTIPLABELCONTENT[14], 100, "%.*f", _Digits, mouseDnAsk);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';

            ttipTextPtr += 2;

            ttipTextPtr = ttipTextPtr + sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", _Digits, mouseDnBid);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';

            ttipTextPtr += 2;

            sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", doubleSignificantPlaces, ((mouseDnAsk - mouseDnBid) * _DigitsMultiplier) * PipsDividerMultiplier);

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;
         }
         else
         {
            ulong actualPreciseTime = timesTabInMs[calculatedIndex];
            ulong timeDiff = 0;
            if (actualPreciseTime >= (ulong)mouseDnTime)
            {
               timeDiff = actualPreciseTime - mouseDnTime;
            }
            else
            {
               timeDiff = mouseDnTime - actualPreciseTime;
            }

            int ticksCount = abs(mouseDnPosX - posX);
            if ((*appSets).tickChartZoom == 1)
               ticksCount *= 2;
            else if ((*appSets).tickChartZoom == 4)
               ticksCount /= 2;

            time_t td = timeDiff / 1000;
            uint days = (((td * 1158050442) >> 32) + td * 49710) >> 32;
            ulong _sc = td - days * 24 * 60 * 60;
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - mn * 60);

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(timeDiff - td * 1000);
            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';
            TOOLTIPLABELCONTENT[14] = 'a';
            TOOLTIPLABELCONTENT[15] = ':';
            TOOLTIPLABELCONTENT[16] = ' ';

            ttipTextPtr = 17 + sprintf_s(&TOOLTIPLABELCONTENT[17], 100, "%.*f", doubleSignificantPlaces, ((asksTab[calculatedIndex] - mouseDnAsk) * _DigitsMultiplier) * PipsDividerMultiplier);

            TOOLTIPLABELCONTENT[ttipTextPtr] = ';';
            TOOLTIPLABELCONTENT[ttipTextPtr + 1] = ' ';
            TOOLTIPLABELCONTENT[ttipTextPtr + 2] = 'b';
            TOOLTIPLABELCONTENT[ttipTextPtr + 3] = ':';
            TOOLTIPLABELCONTENT[ttipTextPtr + 4] = ' ';

            ttipTextPtr += 5;

            ttipTextPtr = ttipTextPtr + sprintf_s(&TOOLTIPLABELCONTENT[ttipTextPtr], 100, "%.*f", doubleSignificantPlaces, ((bidsTab[calculatedIndex] - mouseDnBid) * _DigitsMultiplier) * PipsDividerMultiplier);

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
            refreshRect.bottom = tickChart1.Height() - 1;

            tickChart1.ApplyAlphaBlend(tChHWnd, refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);

            HDC hDc = GetDC(tChHWnd);
            old_pen = (HPEN)SelectObject(hDc, m_simplePen);

            MoveToEx(hDc, mouseDnPosX, mouseDnPosY, NULL);
            LineTo(hDc, posX, posY);

            SelectObject(hDc, old_pen);
            ReleaseDC(tChHWnd, hDc);

            partialToRefresh = true;
         }
         insideArea = true;
      }
      else
      {
         if (defTitleChanged)
            SetWindowText(rootWnd, defaultRootTitle);

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
      int startPoint = (int)tickChart1.GetDataAreaStartPoint();
      if ((uint)(posX - startPoint) < 1000 && (uint)(posY - 1) < 289)
      {
         int ttipTextPtr = 0;

         static int barStartIndex = 0;
         static int mouseDnBarPosX = 0;
         static int mouseDnBarPosY = 0;

         int shift = (posX - startPoint - 999) / (*appSets).barChartCandleWidth;
         int calculatedIndex = chartSearchIndex / (*appSets).barChartTickSize + (shift)-1;

         if (calculatedIndex < 0)
            calculatedIndex = 0;

         calculatedIndex *= (*appSets).barChartTickSize;

         if (!LBUTTON_state)
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

            double pointerPrice = ((286 - posY) * barChart1.GetPointsPerPixel() * _Point) + barChart1.VScaleMin();

            time_t t0 = timesTabInMs[calculatedIndex];
            time_t t1 = t0 / 1000;
            uint days = (((t1 * 1158050442) >> 32) + t1 * 49710) >> 32;
            ulong _sc = t1 - days * 24 * 60 * 60;
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - mn * 60);

            TOOLTIPLABELCONTENT[0] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[1] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[2] = ':';
            TOOLTIPLABELCONTENT[3] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[4] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[5] = ':';
            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[8] = '.';

            uint mil = (uint)(t0 - t1 * 1000);
            TOOLTIPLABELCONTENT[9] = mil / 100 + '0';
            TOOLTIPLABELCONTENT[10] = (mil % 100) / 10 + '0';
            TOOLTIPLABELCONTENT[11] = (mil % 10) + '0';

            TOOLTIPLABELCONTENT[12] = ';';
            TOOLTIPLABELCONTENT[13] = ' ';

            ttipTextPtr = 14 + sprintf_s(&TOOLTIPLABELCONTENT[14], 100, "%.*f", _Digits, pointerPrice);

            SetWindowTextA(rootWnd, TOOLTIPLABELCONTENT);

            defTitleChanged = true;
         }
         else
         {
            int ind1, ind2;
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

            double priceDiff = (abs(mouseDnBarPosY - posY) * barChart1.GetPointsPerPixel()) * PipsDividerMultiplier;

            TOOLTIPLABELCONTENT[0] = 't';
            TOOLTIPLABELCONTENT[1] = 'm';
            TOOLTIPLABELCONTENT[2] = 'D';
            TOOLTIPLABELCONTENT[3] = 'f';
            TOOLTIPLABELCONTENT[4] = ':';
            TOOLTIPLABELCONTENT[5] = ' ';

            time_t td = timeDiff / 1000;
            uint days = (((td * 1158050442) >> 32) + td * 49710) >> 32;
            ulong _sc = td - days * 24 * 60 * 60;
            uint hr = (_sc * 1193047) >> 32;
            _sc -= hr * 3600;
            uint mn = _sc * 71582789 >> 32;
            uint sec = (uint)(_sc - mn * 60);

            TOOLTIPLABELCONTENT[6] = TIMETABLE_A[hr][0];
            TOOLTIPLABELCONTENT[7] = TIMETABLE_A[hr][1];
            TOOLTIPLABELCONTENT[8] = ':';
            TOOLTIPLABELCONTENT[9] = TIMETABLE_A[mn][0];
            TOOLTIPLABELCONTENT[10] = TIMETABLE_A[mn][1];
            TOOLTIPLABELCONTENT[11] = ':';
            TOOLTIPLABELCONTENT[12] = TIMETABLE_A[sec][0];
            TOOLTIPLABELCONTENT[13] = TIMETABLE_A[sec][1];
            TOOLTIPLABELCONTENT[14] = '.';

            uint mil = (uint)(timeDiff - td * 1000);
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
            refreshRect.bottom = barChart1.Height() - 1;

            barChart1.ApplyAlphaBlend(bChHWnd, refreshRect.left, refreshRect.top, (refreshRect.right - refreshRect.left), (refreshRect.bottom - refreshRect.top), refreshRect.left, refreshRect.top);

            HDC hDc = GetDC(bChHWnd);
            old_pen = (HPEN)SelectObject(hDc, m_simplePen);

            MoveToEx(hDc, mouseDnBarPosX, mouseDnBarPosY, NULL);
            LineTo(hDc, posX, posY);

            SelectObject(hDc, old_pen);
            ReleaseDC(bChHWnd, hDc);

            partialToRefresh = true;
         }
      }
      else
      {
         if (defTitleChanged)
            SetWindowText(rootWnd, defaultRootTitle);

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
         if ((appSets->tickChartZoom) & 1) // tickSampleWidth==1
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
         BOOL success;
         uint tickInterv = (uint)GetDlgItemInt(hWnd, IDC_STEP_SIZE_EDT, &success, false);
         if (success)
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
      return;
   BOOL success;
   uint newRange = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, false);
   if (success)
   {
      if (newRange > 1)
      {
         double halfDistance = NormalizeDouble((newRange / 2.0) * _Point, _Digits);
         double meanPrice = NormalizeDouble((bidsTab[chartSearchIndex] + asksTab[chartSearchIndex]) / 2.0, _Digits);
         upRangeLineValue = NormalizeDouble(meanPrice + halfDistance, _Digits);
         downRangeLineValue = NormalizeDouble(meanPrice - halfDistance, _Digits);

         char charBuffer[50];
         sprintf_s(charBuffer, "%.*f", doubleSignificantPlaces, (abs(upRangeLineValue - downRangeLineValue) * _DigitsMultiplier) * PipsDividerMultiplier);

         size_t len = strlen(charBuffer);

         charBuffer[len] = ' ';
         charBuffer[len + 1] = 'p';
         charBuffer[len + 2] = 'i';
         charBuffer[len + 3] = 'p';
         charBuffer[len + 4] = 's';
         charBuffer[len + 5] = '\0';

         SetDlgItemTextA(hWnd, IDC_INFOLABEL, charBuffer);

         tickChart1.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tickChart1.VScaleParams(upRangeLineValue, downRangeLineValue, 10, true);

         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
      }
   }
}
void CTickChartModule::SetBarTickSizeBtnClicked()
{
   BOOL success;
   uint newVal = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, false);
   if (success)
   {
      if (newVal > 1)
      {
         (*appSets).barChartTickSize = newVal;
         if (chartSearchIndex > -1)
         {
            RecalculateBarChart(chartSearchIndex, false);
            barChart1.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
            barChart1.UpdateChart(true);
         }
         else
            barChart1.SetBarChartTickSize((*appSets).barChartTickSize, false);
      }
   }
}
void CTickChartModule::RecalculateBarChart(int endInd, bool redraw)
{
   barChartTickSizeCounter = 0;
   barChart1.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0);

   barChartHigh = DBL_MAX * -1.0;
   barChartLow = DBL_MAX;

   int i = 0;

   for (; i < endInd; i++)
   {
      barChartTickSizeCounter++;
      if (barChartHigh < bidsTab[i])
         barChartHigh = bidsTab[i];

      if (barChartLow > bidsTab[i])
         barChartLow = bidsTab[i];

      if (barChartTickSizeCounter == (*appSets).barChartTickSize)
      {
         ulong parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
         barChart1.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
         barChartTickSizeCounter = 0;
      }
      barChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
   }
   barChart1.SetBarChartTickSize((*appSets).barChartTickSize, redraw);
}
void CTickChartModule::MoveRangeUpBtnClicked()
{
   if (seriesIndex == -1)
      return;
   BOOL success;
   uint valueToMove = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, false);
   if (success)
   {
      if (valueToMove > 0)
      {
         upRangeLineValue = NormalizeDouble(upRangeLineValue + valueToMove * _Point, _Digits);
         if ((*appSets).freezeRanges)
            downRangeLineValue = NormalizeDouble(downRangeLineValue + valueToMove * _Point, _Digits);

         tickChart1.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tickChart1.VScaleParams(upRangeLineValue, downRangeLineValue, 10);

         PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -MOVE_RANGE_UP);
      }
   }
}
void CTickChartModule::MoveRangeDownBtnClicked()
{
   if (seriesIndex == -1)
      return;
   BOOL success;
   uint valueToMove = (uint)GetDlgItemInt(hWnd, IDC_MAIN_EDT, &success, false);
   if (success)
   {
      if (valueToMove > 0)
      {
         upRangeLineValue = NormalizeDouble(upRangeLineValue - valueToMove * _Point, _Digits);
         if ((*appSets).freezeRanges)
            downRangeLineValue = NormalizeDouble(downRangeLineValue - valueToMove * _Point, _Digits);

         tickChart1.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
         tickChart1.VScaleParams(upRangeLineValue, downRangeLineValue, 10);

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
      return;
   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -ADD_SIGNED_LEVEL);
}
void CTickChartModule::DeleteSignedLevelsBtnClicked()
{
   if (seriesIndex == -1)
      return;
   if (MessageBox(hWnd, L"Delete all '_TT' HLine objects?", L"Deleting HLine objects", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
   {
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -DELETE_ALL_SIGNED_LEVELS);
   }
}
void CTickChartModule::TickChartZoomChanged()
{
   tickChart1.SetTickSampleWidth((*appSets).tickChartZoom, false);

   if (seriesIndex == -1)
      return;

   tickChart1.UpdateChart();
}
void CTickChartModule::BarChartCandleWidthChanged()
{
   barChart1.SetBarChartBarWidth((*appSets).barChartCandleWidth, seriesIndex > -1);
}
void CTickChartModule::TickChartTimeSepChanged()
{
   tickChart1.SetInterval((*appSets).tickChartTimeSep, seriesIndex > -1);
}
void CTickChartModule::BarChartTimeSepChanged()
{
   barChart1.SetInterval((*appSets).barChartTimeSep, seriesIndex > -1);
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
   tickChart1.RefreshWindow();
}
void CTickChartModule::RefreshTickChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   tickChart1.RefreshWindow(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CTickChartModule::RefreshBarChartWindow()
{
   barChart1.RefreshWindow();
}
void CTickChartModule::RefreshBarChartWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   barChart1.RefreshWindow(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CTickChartModule::AskLineVisChanged()
{
   tickChart1.ChartVisibility(0, (int)(*appSets).askLineVis, seriesIndex > -1);
}
void CTickChartModule::BidLineVisChanged()
{
   tickChart1.ChartVisibility(1, (int)(*appSets).bidLineVis, seriesIndex > -1);
};
void CTickChartModule::SignedLevelsVisChanged()
{
   if (seriesIndex == -1)
      return;

   if ((*appSets).signedLevelsVis)
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SHOW_SIGNED_LEVELS);

   tickChart1.ShowSignedLevels((*appSets).signedLevelsVis, !(*appSets).signedLevelsVis);
   barChart1.ShowSignedLevels((*appSets).signedLevelsVis, !(*appSets).signedLevelsVis);
}
void CTickChartModule::UpdateVisLevels(bool updateChart)
{
   tickChart1.ShowSignedLevels((*appSets).signedLevelsVis, updateChart);
   barChart1.ShowSignedLevels((*appSets).signedLevelsVis, updateChart);
}
void CTickChartModule::AutoMovingRangeChanged()
{
}
void CTickChartModule::MProfileAskVisChanged()
{
   if (_Digits <= 5)
   {
      tickChart1.ShowMProfileDataAsk((*appSets).mProfileAskVis, seriesIndex > -1);
      barChart1.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, seriesIndex > -1);
   }
}
void CTickChartModule::MProfileBidVisChanged()
{
   if (_Digits <= 5)
   {
      tickChart1.ShowMProfileDataBid((*appSets).mProfileBidVis, seriesIndex > -1);
      barChart1.ShowMProfileData((*appSets).mProfileAskVis || (*appSets).mProfileBidVis, seriesIndex > -1);
   }
}
void CTickChartModule::TickChartVisChanged()
{
   tickChart1.ShowMainPlot((*appSets).tickChartVis, seriesIndex > -1);
}
void CTickChartModule::BarChartVisChanged()
{
   barChart1.ShowMainPlot((*appSets).barChartVis, seriesIndex > -1);
}
void CTickChartModule::TimeParameterVisChanged()
{
   tickChart1.ShowTimeParameter((*appSets).timeParamVis, seriesIndex > -1);
   barChart1.ShowTimeParameter((*appSets).timeParamVis, seriesIndex > -1);
}
void CTickChartModule::CumulativeAskVisChanged()
{
   tickChart1.ShowCumulativeDataAsk((*appSets).cumulativeAskVis, seriesIndex > -1);
}
void CTickChartModule::CumulativeBidVisChanged()
{
   tickChart1.ShowCumulativeDataBid((*appSets).cumulativeBidVis, seriesIndex > -1);
}
void CTickChartModule::DistanceVisChanged()
{
   if (EnableSpeedStats)
      tickChart1.ShowTravelledDistance((*appSets).distanceVis, seriesIndex > -1);
}
void CTickChartModule::RoadVisChanged()
{
   if (EnableSpeedStats)
      tickChart1.ShowTravelledRoad((*appSets).roadVis, seriesIndex > -1);
}
void CTickChartModule::TicksArrivedVisChanged()
{
   if (EnableSpeedStats)
      tickChart1.ShowTicksArrived((*appSets).ticksArrivedVis, seriesIndex > -1);
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
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SHOW_ORDERS);
   else
   {
      tickChart1.ShowOrderPoint(false, seriesIndex > -1);
      barChart1.ShowOrderPoint(false, seriesIndex > -1);
   }
}
void CTickChartModule::ColorTimeParamChanged()
{
   tickChart1.ColorTimeParameter((*appSets).colorTimeParam, seriesIndex > -1);
   barChart1.ColorTimeParameter((*appSets).colorTimeParam, seriesIndex > -1);
}
void CTickChartModule::ZoomTimeParamChanged()
{
   UpdateBiggerBarsData(seriesIndex > -1);
}

bool CTickChartModule::SaveTicksClicked(LPCTSTR pszFileName)
{
   HANDLE hFile;
   bool bSuccess = false;

   hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize;

      std::string strContent = "";
      char temp[1000];

      for (int i = 0; i < seriesIndex; i++)
      {
         _i64toa_s(timesTabInMs[i] / 1000, temp, 100, 10);
         strContent.append(temp);
         strContent += ',';
         _itoa_s(timesTabInMs[i] % 1000, temp, 100, 10);
         strContent.append(temp);
         strContent += ',';

         sprintf_s(temp, 100, "%.*f", _Digits, asksTab[i]);
         strContent.append(temp);
         strContent += ',';
         sprintf_s(temp, 100, "%.*f", _Digits, bidsTab[i]);
         strContent.append(temp);
         strContent += '\n';
      }
      dwFileSize = (DWORD)strContent.size();
      DWORD dwWritten;

      if (WriteFile(hFile, strContent.c_str(), dwFileSize, &dwWritten, NULL))
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
      return -1;

   Gdiplus::ImageCodecInfo *pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
   if (pImageCodecInfo == nullptr)
      return -1;

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
   HANDLE hFile;
   bool bSuccess = false;

   hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize;

      dwFileSize = GetFileSize(hFile, NULL);
      if (dwFileSize != 0xFFFFFFFF)
      {
         char *pszFileText = NULL;
         pszFileText = new char[dwFileSize];

         if (pszFileText != NULL)
         {
            DWORD dwRead;

            if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL))
            {
               long long *lngPtr = (long long *)pszFileText;
               double *dblPtr = (double *)pszFileText;

               dataSize = (int)lngPtr[0];

               if (dataSize > 0)
               {
                  if (timesTabInMs != NULL)
                     delete[] timesTabInMs;
                  if (bidsTab != NULL)
                     delete[] bidsTab;
                  if (asksTab != NULL)
                     delete[] asksTab;
                  if (realTempoValsTab != NULL)
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
                        realTempoValsTab[dataIterator] = (int)(timesTabInMs[dataIterator] - timesTabInMs[dataIterator - 1]);
                  }
                  SetDlgItemInt(hWnd, IDC_INFOLABEL, (UINT)dataIterator, false);
                  SetDlgItemText(hWnd, IDC_DATELABEL, L"0000.00.00");
                  TimeSepVLine = timesTabInMs[0];
               }
            }
         }
         delete pszFileText;
      }
      CloseHandle(hFile);
   }
   return (bSuccess);
}
void CTickChartModule::ResetData()
{
   seriesIndex = -1;
   barChartHigh = DBL_MAX * -1.0;
   barChartLow = DBL_MAX;

   barChartTickSizeCounter = 0;
   chartSearchIndex = -1;

   tickChart1.FillSeries(last_tick.ask, last_tick.bid, 0, 0, 0, 0);
   barChart1.FillSeries(last_tick.bid, last_tick.bid, last_tick.bid, last_tick.bid, 0, 0);
}
void CTickChartModule::UpdateBiggerBarsData(bool updateChart)
{
   uint newMultiplier = 1;
   if ((*appSets).zoomTimeParam)
      newMultiplier = 4;

   tickChart1.BiggerBarsData(newMultiplier, updateChart);
   barChart1.BiggerBarsData(newMultiplier, updateChart);
}
int CTickChartModule::OnTimer()
{
   int nextInterval = 0;

   if ((*appSets).realTempo)
   {
      if (chartSearchIndex > -1)
      {
         LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
         LARGE_INTEGER Frequency;
         QueryPerformanceFrequency(&Frequency);
         QueryPerformanceCounter(&StartingTime);

         StepForwardBtnClicked();

         QueryPerformanceCounter(&EndingTime);
         ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
         long long interval = static_cast<long long>(ElapsedMicroseconds.QuadPart * 1000 / (Frequency.QuadPart));

         nextInterval = realTempoValsTab[chartSearchIndex] - (int)(interval);
         if ((appSets->tickChartZoom) & 1) // tickSampleWidth==1 Step forward == 2, to avoid tick chart flickering
         {
            if (chartSearchIndex < seriesIndex)
            {
               nextInterval += realTempoValsTab[chartSearchIndex + 1];
            }
         }
         if (nextInterval <= 0)
            nextInterval = 1;
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
   tickChart1.AppendSignedLevels(levels, levelsSize, descriptions, descrSize, false);
   barChart1.AppendSignedLevels(levels, levelsSize, descriptions, descrSize, false);
   if (update && seriesIndex > -1)
   {
      PostMessageA(rootWnd, UPDATE_CHARTS, 0, 1);
   }
}
void CTickChartModule::AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size, const bool update)
{
   tickChart1.AppendTransactionsPoints(transactions, descriptions, size);
   barChart1.AppendTransactionsPoints(transactions, descriptions, size);
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
bool CTickChartModule::GetRangeValData(double &upRangeVal, double &downRangeVal)
{
   upRangeVal = upRangeLineValue;
   downRangeVal = downRangeLineValue;
   return (true);
}
bool CTickChartModule::RangeLineDragged(double &newValue, int index)
{
   if (seriesIndex == -1)
      return (true);

   if (index == 0)
   {
      if ((*appSets).freezeRanges)
      {
         double difference = NormalizeDouble(newValue - upRangeLineValue, _Digits);
         downRangeLineValue = downRangeLineValue + difference;
      }
      upRangeLineValue = newValue;
   }
   else
   {
      if ((*appSets).freezeRanges)
      {
         double difference = NormalizeDouble(newValue - downRangeLineValue, _Digits);
         upRangeLineValue = upRangeLineValue + difference;
      }
      downRangeLineValue = newValue;
   }
   tickChart1.MoveMarketProfileRange(downRangeLineValue, upRangeLineValue);
   tickChart1.VScaleParams(upRangeLineValue, downRangeLineValue, 10, false);

   PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -MOVE_RANGE_UP);
   PostMessageA(rootWnd, UPDATE_CHARTS, 1, 1);
   return (true);
}
bool CTickChartModule::TimeSepVLineDragged(__int64 &newValue)
{
   if (seriesIndex == -1)
      return (true);

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
   _Point = sets.pointValue;
   _Digits = sets.digitsCount;
   _DigitsMultiplier = POWER_OF_10[_Digits];
   ExpandDateRange = sets.expandDateRange;
   ExcludePremarketData = sets.excludePremarketData;
   EnableSpeedStats = sets.enableSpeedStats;

   memcpy(TOOLTIPLABELCONTENT, date, dateChars);

   last_tick.ask = sets.initAsk;
   last_tick.bid = sets.initBid;
   last_tick.time = sets.initTime;

   time_t midnightDt = (sets.initTime / (3600 * 24)) * (3600 * 24);

   tickChart1.SetColorMode(DarkMode);
   barChart1.SetColorMode(DarkMode);

   return (true);
}
bool CTickChartModule::TickDataLoaded(MqlTick tckArray[], int arrSize, char *date, int dateChars)
{
   seriesIndex = -1;
   chartSearchIndex = -1;
   int _dstInd = 0, _srcInd = 0;

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
      PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 1);

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
            realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
         else
            realTempoValsTab[0] = (*appSets).timerInterval;

         int stopIdx = (seriesIndex + arrSize - 1) < dataSize ? arrSize : dataSize - seriesIndex;
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
      PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 1);
   return (true);
}
bool CTickChartModule::OnNewTick(MqlTick &lastTick)
{
   if (readyToUse)
   {
      if (seriesIndex < dataSize - 1)
         seriesIndex++;

      // Critical error: throws assembly code in the MT5 terminal
      timesTabInMs[seriesIndex] = lastTick.time_msc;
      bidsTab[seriesIndex] = lastTick.bid;
      asksTab[seriesIndex] = lastTick.ask;
      if (seriesIndex > 0)
         realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
      else
         realTempoValsTab[0] = (*appSets).timerInterval;

      if ((*appSets).chartAutoScroll)
      {
         if ((appSets->tickChartZoom) & 1) // tickSampleWidth==1
         {
            if (seriesIndex & 1) // seriesIndex&1 - when index is odd (update every 2 ticks)
               PostMessageA(rootWnd, AUTO_SCROLL_UPDATE, 0, 0);
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
            seriesIndex++;
         timesTabInMs[seriesIndex] = tckArray[0].time_msc;
         bidsTab[seriesIndex] = tckArray[0].bid;
         asksTab[seriesIndex] = tckArray[0].ask;
         if (seriesIndex > 0)
            realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
         else
            realTempoValsTab[0] = (*appSets).timerInterval;
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
               realTempoValsTab[seriesIndex] = (int)(timesTabInMs[seriesIndex] - timesTabInMs[seriesIndex - 1]);
            else
               realTempoValsTab[0] = (*appSets).timerInterval;
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

      tickChart1.FillSeries(asksTab[0], bidsTab[0], 0, 0, 0, 0);
      barChart1.FillSeries(bidsTab[0], bidsTab[0], bidsTab[0], bidsTab[0], 0, 0, false);
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
            bidChange = 1;
         else if (bidsTab[i] < bidsTab[i - 1])
            bidChange = -1;

         if (asksTab[i] > asksTab[i - 1])
            askChange = 1;
         else if (asksTab[i] < asksTab[i - 1])
            askChange = -1;

         if (EnableSpeedStats)
         {
            for (int j = i - 1; j > 0; j--)
            {
               wholeRoad += abs(bidsTab[j] - bidsTab[j - 1]) * _DigitsMultiplier;
               if ((long)((timesTabInMs[i - 1] - timesTabInMs[j]) / 1000) >= noOfSecondsForCalc)
               {
                  priceDistance = (short)NormalizeDouble((bidsTab[i - 1] - bidsTab[j]) * _DigitsMultiplier, 0);
                  ticksElapsed = short(i - 1 - j);
                  break;
               }
            }
         }
      }
      tickChart1.AppendPricesTimeAndParameters(asksTab[i], bidsTab[i], timesTabInMs[i] / 1000, parameter, askChange, bidChange, (short)wholeRoad, priceDistance, ticksElapsed);

      if (barChartHigh < bidsTab[i])
         barChartHigh = bidsTab[i];

      if (barChartLow > bidsTab[i])
         barChartLow = bidsTab[i];

      if (barChartTickSizeCounter == (*appSets).barChartTickSize)
      {
         parameter = timesTabInMs[i] - timesTabInMs[i - (*appSets).barChartTickSize + 1];
         barChart1.AppendPricesTimeAndParameters(bidsTab[i - (*appSets).barChartTickSize + 1], bidsTab[i], barChartHigh, barChartLow, timesTabInMs[i - (*appSets).barChartTickSize + 1] / 1000, parameter);

         barChartHigh = DBL_MAX * -1.0;
         barChartLow = DBL_MAX;
         barChartTickSizeCounter = 0;
         updateBarChart = true;
      }

      tickChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
      barChart1.UpdateMarketProfile(asksTab[i], bidsTab[i]);
   }

   if ((*appSets).autoMovingRange)
   {
      double halfDistance = (upRangeLineValue - downRangeLineValue) / 2.0;
      (*reinterpret_cast<unsigned long long *>(&halfDistance)) &= 0xffffffffffffffff >> 1;
      double meanPrice = (bidsTab[chartSearchIndex] + asksTab[chartSearchIndex]) / 2.0;
      double newHPrice = NormalizeDouble(meanPrice + halfDistance, _Digits);
      double newLPrice = NormalizeDouble(meanPrice - halfDistance, _Digits);

      upRangeLineValue = newHPrice;
      downRangeLineValue = newLPrice;
      tickChart1.VScaleParams(newHPrice, newLPrice, 10, false);
      tickChart1.MoveMarketProfileRange(newLPrice, newHPrice);
      PostMessage(TerminalParentChartHWnd, WM_LBUTTONUP, 0, -SET_RANGE);
   }
   tickChart1.UpdateChart(forceVScaleUpdate);

   barChart1.SetActualBidPrice(bidsTab[chartSearchIndex], barChartHigh, barChartLow);
   if (updateBarChart)
      barChart1.UpdateChart(forceVScaleUpdate);
   else
      barChart1.UpdateCurrentPriceLevel();
}
bool CTickChartModule::UpdateCharts(byte mode, bool forceVScaleUpdate)
{
   switch (mode)
   {
   case 0:
   {
      tickChart1.UpdateChart(forceVScaleUpdate);
      barChart1.UpdateChart(forceVScaleUpdate);
   }
   break;
   case 1:
      tickChart1.UpdateChart(forceVScaleUpdate);
      break;
   // mode==2
   default:
      barChart1.UpdateChart(forceVScaleUpdate);
      break;
   }
   return (true);
}