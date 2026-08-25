#include "StdAfx.h"
#include "BarChart.h"

CBarChart::CBarChart(void)
{
   ShowFlags(FLAG_SHOW_LEGEND | FLAGS_SHOW_SCALES | FLAG_SHOW_GRID);

   DarkMode = true;

   workSpaceChartHeightInPx = 270;
   pointsPerPx = 0;
   mDataAreaStartPoint = 0;

   initialized = false;
   interval = 5;
   mainPlotVis = true;
   barsDataMultiplier = 1;
   barChartTickSize = 30;

   mProfileStartIndex = -1;
   mProfileSize = 0;
   mProfileSizeFactor = 1;
   totalMProfileSize = 100000;

   drawVerticalGrid = false;

   mProfileDataVis = false;

   seriesSize = 10000;
   chartWidthInSamples = 1000;
   seriesPointer = chartWidthInSamples - 2;
   barChartBarWidth = 4;

   extremumCount = chartWidthInSamples;
   extremumStartIndex = 0;

   times = new time_t[seriesSize];
   memset(times, 0, seriesSize * sizeof(time_t));

   openPrices = new double[seriesSize];
   memset(openPrices, 0, seriesSize * sizeof(double));
   closePrices = new double[seriesSize];
   memset(closePrices, 0, seriesSize * sizeof(double));
   highPrices = new double[seriesSize];
   memset(highPrices, 0, seriesSize * sizeof(double));
   lowPrices = new double[seriesSize];
   memset(lowPrices, 0, seriesSize * sizeof(double));
   timeParameters = new ulong[seriesSize];
   memset(timeParameters, 0, seriesSize * sizeof(ulong));

   mProfileData = new int[totalMProfileSize];
   memset(mProfileData, 0, seriesSize * sizeof(int));

   isCalendarEvents = false;
   showCalendarEvents = false;
   isSignedLevelsDescriptions = false;
   pipsDivider = 1;

   transactionsDescriptions = NULL;
   transactionsTab = NULL;
   transactionsTabSize = 0;
   signedLevelsDescriptions = NULL;

   signedLevelsArraySize = 0;
   signedLevels = NULL;

   calendarEvents = NULL;
   calendarEventsTabSize = 0;

   SetColorMode(DarkMode);

   for (int i = 0; i < 256; i++)
   {
      timeParamColors[i][0] = (uint)XRGB_gdi(255 - i, 255 - i, 255 - i);
      timeParamColors[i][1] = (uint)XRGB_gdi(255 - i, 0, 0);
   }

   rescaledMProfileTabSize = 1;
   rescaledMProfile = new double[rescaledMProfileTabSize];

   interval_idx = 4;             // for 60min
   intervals_quot[0] = 71582789; //(unsigned long long)ceil(4294967296/(1.0*60));
   intervals_quot[1] = 14316558; //(unsigned long long)ceil(4294967296/(5.0*60));
   intervals_quot[2] = 4772186;  //(unsigned long long)ceil(4294967296/(15.0*60));
   intervals_quot[3] = 2386093;  //(unsigned long long)ceil(4294967296/(30.0*60));
   intervals_quot[4] = 1193047;  //(unsigned long long)ceil(4294967296/(60.0*60));

   actualBidPrice = 0.0;
   previousBidPrice = 0.0;
   actualBidHigh = DBL_MAX * -1.0;
   actualBidLow = DBL_MAX;
   lastChartPriceMax = DBL_MAX;
   lastChartPriceMin = -DBL_MAX;
}

CBarChart::~CBarChart(void)
{
   delete[] times;
   if (transactionsTab != NULL)
      delete[] transactionsTab;
   if (calendarEvents != NULL)
      delete[] calendarEvents;
   if (signedLevelsDescriptions != NULL)
      delete[] signedLevelsDescriptions;
   if (signedLevels != NULL)
      delete[] signedLevels;
   delete[] mProfileData;
   delete[] openPrices;
   delete[] closePrices;
   delete[] highPrices;
   delete[] lowPrices;
   delete[] timeParameters;
   if (transactionsDescriptions != NULL)
      delete[] transactionsDescriptions;

   delete[] rescaledMProfile;
}
bool CBarChart::Create(HWND hWnd, const int width, const int height, double pointValue, int digits)
{
   if (!ChartCanvas::Create(hWnd, width, height))
      return (false);

   _Digits = digits;
   _Point = pointValue;
   _DigitsMultiplier = POWER_OF_10[digits];

   return (true);
}
void CBarChart::SetColorMode(const bool value)
{
   DarkMode = value;
   if (DarkMode)
   {
      signedLevelsObjColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(10, 10, 10);
      signedLevelsTextColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(150, 150, 150);
      timeParameterObjColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(62, 225, 62);
      mProfileColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(196, 196, 255);
      bullCandleColor = (uint)XRGB_gdi(0, 0, 0);
      bearCandleColor = (uint)XRGB_gdi(0, 255, 0);
      timeIntervalTextColor = (uint)XRGB_gdi(255, 255, 255);
      timeIntervalObjColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(212, 212, 212);
      verticalGridColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(230, 230, 230);
      orderPointBuyColor = (uint)XRGB_gdi(51, 51, 226);
      orderPointSellColor = (uint)XRGB_gdi(226, 51, 51);
      orderPointCloseColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(218, 165, 32);
      orderPointLineColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(212, 212, 212);
      orderPointTextColor = (uint)XRGB_gdi(255, 255, 255);
      calendarEventCircleColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(128, 128, 128);
   }
   else
   {
      signedLevelsObjColor = (uint)XRGB_gdi(10, 10, 10);
      signedLevelsTextColor = (uint)XRGB_gdi(150, 150, 150);
      timeParameterObjColor = (uint)XRGB_gdi(62, 225, 62);
      mProfileColor = (uint)XRGB_gdi(196, 196, 255);
      bullCandleColor = (uint)XRGB_gdi(255, 255, 255);
      bearCandleColor = (uint)XRGB_gdi(0, 0, 0);
      timeIntervalTextColor = (uint)XRGB_gdi(0, 0, 0);
      timeIntervalObjColor = (uint)XRGB_gdi(212, 212, 212);
      verticalGridColor = (uint)XRGB_gdi(230, 230, 230);
      orderPointBuyColor = (uint)XRGB_gdi(0, 0, 255);
      orderPointSellColor = (uint)XRGB_gdi(255, 0, 0);
      orderPointCloseColor = (uint)XRGB_gdi(218, 165, 32);
      orderPointLineColor = (uint)XRGB_gdi(212, 212, 212);
      orderPointTextColor = (uint)XRGB_gdi(0, 0, 0);
      calendarEventCircleColor = (uint)XRGB_gdi(128, 128, 128);
   }
}
void CBarChart::AppendPricesTimeAndParameters(const double open, const double close, const double high, const double low, const time_t time, const ulong timeParameter)
{
   seriesPointer++;
   if (seriesPointer == seriesSize)
   {
      memmove(openPrices, &openPrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(closePrices, &closePrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(highPrices, &highPrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(lowPrices, &lowPrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(times, &times[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(time_t));
      memmove(timeParameters, &timeParameters[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(ulong));

      seriesPointer = chartWidthInSamples;
   }

   openPrices[seriesPointer] = open;
   closePrices[seriesPointer] = close;
   highPrices[seriesPointer] = high;
   lowPrices[seriesPointer] = low;
   times[seriesPointer] = time;
   if (timeParameter != 0)
      timeParameters[seriesPointer] = ((1024UL * 1000UL) / timeParameter) + 1;
   else
      timeParameters[seriesPointer] = 1;

   initialized = true;
}
void CBarChart::FillSeries(const double open, const double close, const double high, const double low, const time_t time, const ulong timeParameter, const bool update)
{
   for (int i = 0; i < seriesSize; i++)
   {
      openPrices[i] = open;
      closePrices[i] = close;
      highPrices[i] = high;
      lowPrices[i] = low;
      times[i] = time;
      timeParameters[i] = timeParameter;
   }

   long long *tab1 = (long long *)(mProfileData);
   for (int i = 0; i < totalMProfileSize / 2; i++)
   {
      tab1[i] = 0;
   }

   seriesPointer = chartWidthInSamples - 2;
   initialized = false;

   if (update)
      Redraw();
}
void CBarChart::UpdateMarketProfile(const double askPrice, const double bidPrice)
{
   double midPrice = (askPrice + bidPrice) / 2.0;
   int midInd = (int)(midPrice * _DigitsMultiplier + 0.5) % 100000;

   mProfileData[midInd]++;
}
void CBarChart::AppendSignedLevels(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update)
{
   isSignedLevelsDescriptions = false;

   if (levelsSize > 0)
   {
      if (descrSize > 0)
      {
         if (descrSize != levelsSize)
            return;

         if (signedLevelsArraySize != descrSize)
         {
            delete[] signedLevelsDescriptions;
            signedLevelsDescriptions = new char[descrSize][100];
         }
         memcpy(signedLevelsDescriptions, descriptions, descrSize * 100 * sizeof(char));
         isSignedLevelsDescriptions = true;
      }

      if (signedLevelsArraySize != levelsSize)
      {
         delete[] signedLevels;
         signedLevels = new double[levelsSize];
      }
      memcpy(signedLevels, levels, levelsSize * sizeof(double));
      signedLevelsArraySize = levelsSize;
   }
   else
   {
      if (signedLevelsDescriptions != NULL)
      {
         delete[] signedLevelsDescriptions;
         signedLevelsDescriptions = NULL;
      }
      if (signedLevels != NULL)
      {
         delete[] signedLevels;
         signedLevels = NULL;
      }
      signedLevelsArraySize = 0;
   }
   if (update)
   {
      vScaleParamsChanged = true;
      Redraw();
   }
}
void CBarChart::AppendCalendarEvents(CalendarEvent clEvents[], int tabSize, const bool update)
{
   if (tabSize > 0)
   {
      calendarEvents = new CalendarEvent[tabSize];
      memcpy(calendarEvents, clEvents, tabSize * sizeof(*calendarEvents));

      // For Security reason
      for (int i = 0; i < tabSize; i++)
      {
         calendarEvents[i].importance = calendarEvents[i].importance % 10;
      }
      calendarEventsTabSize = tabSize;
      isCalendarEvents = true;
   }
   if (update)
      Redraw();
}
void CBarChart::AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size, const bool update)
{
   if (size > 0)
   {
      if (size != transactionsTabSize)
      {
         delete[] transactionsTab;
         transactionsTab = new long long[size][4];

         if (transactionsDescriptions != NULL)
            delete[] transactionsDescriptions;

         transactionsDescriptions = new char[size][64];
      }
      memcpy(transactionsTab, transactions, size * 4 * sizeof(long long));
      memcpy(transactionsDescriptions, descriptions, size * 64 * sizeof(char));
      transactionsTabSize = size;
      isOrdersPoints = true;
   }
   else
   {
      isOrdersPoints = false;
   }
   vScaleParamsChanged = true;

   if (update)
      Redraw();
}
void CBarChart::SetActualBidPrice(const double currentPrice, const double currentHigh, const double currentLow)
{
   previousBidPrice = actualBidPrice;
   actualBidPrice = currentPrice;
   actualBidHigh = currentHigh;
   actualBidLow = currentLow;
}
void CBarChart::UpdateCurrentPriceLevel(void)
{
   if (actualBidHigh > lastChartPriceMax || actualBidLow < lastChartPriceMin)
   {
      UpdateChart(false);
   }
   else
   {
      static int _lastPrLvlPt = -1;
      int _prLvlPt = 0;

      // First check if redrawing current prive level is necessary
      // if level on the chart is the same as last one: no need to chart update
      if (actualBidPrice != 0)
      {
         double _prLvl = actualBidPrice;
         if (m_v_scale_min > 0)
         {
            _prLvl -= m_v_scale_min;
         }
         _prLvlPt = (int)(m_y_0 - _prLvl * m_scale_y + 0.5);
      }
      if (_prLvlPt != _lastPrLvlPt)
      {
         int _cl_prLvlPt = 0, _bar_prHighPt = 0, _bar_prLowPt = 0;
         int x1 = 1072;
         // clear previous price level sign
         if (previousBidPrice != 0)
         {
            double _prlvl = previousBidPrice;
            if (m_v_scale_min > 0)
            {
               _prlvl -= m_v_scale_min;
            }
            _cl_prLvlPt = (int)(m_y_0 - _prlvl * m_scale_y + 0.5);
            SafeSortedLineHorizontal(x1, x1 + 6, _cl_prLvlPt, ColorBackground());
         }
         // draw current bar on chart
         if (actualBidLow != DBL_MAX)
         {
            double _prHigh = actualBidHigh;
            double _prLow = actualBidLow;
            if (m_v_scale_min > 0)
            {
               _prHigh -= m_v_scale_min;
               _prLow -= m_v_scale_min;
            }
            _bar_prHighPt = (int)(m_y_0 - _prHigh * m_scale_y + 0.5);
            _bar_prLowPt = (int)(m_y_0 - _prLow * m_scale_y + 0.5);
            SafeSortedLineVertical(x1, _bar_prHighPt, _bar_prLowPt, bearCandleColor);
         }
         // draw current price level Sign on chart
         SafeSortedLineHorizontal(x1, x1 + 6, _prLvlPt, bearCandleColor);
         // Note about plot indexing values: x increasing from left to right,
         // y increasing from top to bottom (as oposite to price values on chart)
         if (_cl_prLvlPt > _bar_prLowPt)
            _bar_prLowPt = _cl_prLvlPt;
         else if (_cl_prLvlPt < _bar_prHighPt)
            _bar_prHighPt = _cl_prLvlPt;

         Update(x1, _bar_prHighPt, 7, _bar_prLowPt - _bar_prHighPt + 1, x1, _bar_prHighPt);
      }
      _lastPrLvlPt = _prLvlPt;
   }
}
void CBarChart::UpdateChart(bool vScaleParChanged)
{
   if (initialized)
   {
      if (vScaleParChanged)
         vScaleParamsChanged = vScaleParChanged;

      extremumStartIndex = seriesPointer - extremumCount + 1;

      double maximum = MaxInArray2(highPrices, extremumStartIndex, extremumCount);
      double minimum = MinInArray2(lowPrices, extremumStartIndex, extremumCount);

      if (actualBidHigh > maximum)
         maximum = actualBidHigh;
      if (actualBidLow < minimum)
         minimum = actualBidLow;

      if (maximum != lastChartPriceMax || minimum != lastChartPriceMin)
      {
         double maxVal = NormalizeDouble(maximum + 10 * _Point, _Digits - 1);
         double minVal = NormalizeDouble(minimum - 10 * _Point, _Digits - 1);

         int noOfLevels = (int)((maxVal - minVal) * _DigitsMultiplier / 10.0);
         int limit = noOfLevels / 10;
         if (noOfLevels % 10 != 0)
            limit += 1;

         double maxEvenVal = minVal + (double)(limit * 100.0 * _Point);

         if (m_v_scale_max != maxEvenVal || m_v_scale_min != minVal)
         {
            vScaleParamsChanged = true;
            pointsPerPx = ((maxEvenVal - minVal) * _DigitsMultiplier) / workSpaceChartHeightInPx;
            VScaleParams(maxEvenVal, minVal, 10, false);
         }

         mProfileStartIndex = ((int)(minVal * _DigitsMultiplier + 0.5) % 100000);
         int endInd = ((int)(maxEvenVal * _DigitsMultiplier + 0.5) % 100000);
         mProfileSize = endInd - mProfileStartIndex;

         if (mProfileSize > 1)
            mProfileSizeFactor = 2147483648 / (mProfileSize - 1);
         else
            mProfileSizeFactor = 2147483648 / (mProfileSize);
      }
      lastChartPriceMax = maximum;
      lastChartPriceMin = minimum;

      Redraw();
   }
}
void CBarChart::RefreshWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   Update(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CBarChart::RefreshWindow()
{
   Update();
}
void CBarChart::DrawChart(void)
{
   if (initialized)
   {
      DrawData(0);
   }
}
void CBarChart::DrawData(const uint index)
{
   mDataAreaStartPoint = m_data_area.left;

   int dx = barChartBarWidth;
   int x = m_data_area.left + 1;

   RECT rct;
   if (vScaleParamsChanged)
      DrawBitTimeSepStamp_09(DESCRIPTION, 10, 2, 2, 1, 100);

   if (mProfileDataVis)
   {
      DrawMProfile(extremumStartIndex);
   }
   if (mSignedLevelsVis && signedLevelsArraySize > 0)
   {
      DrawSignedLevels();
   }

   double y_scale_shift = 0;
   if (m_v_scale_min > 0)
      y_scale_shift = m_v_scale_min;

   ulong tmParamMax = 0;
   ulong tmParamMin = 0;
   ulong tmParamFactor = 0;

   if (mTimeParameterVis)
   {
      MinMax(timeParameters, extremumStartIndex, extremumCount, tmParamMin, tmParamMax);
      if (tmParamMax == tmParamMin)
         ++tmParamMax;

      // 2^21 = 2097152
      // 255: max index from colorArray
      tmParamFactor = ((2097152 * 255) / (tmParamMax - tmParamMin)) + 1;
   }
   uint tmParamFigureHight = (int)(10 * barsDataMultiplier);

   int i = seriesPointer - extremumCount + 1;

   int calEvStartInd = -1;
   int calEvEndInd = -1;

   if (isCalendarEvents & showCalendarEvents)
   {
      int firstValidRateIndex = i;
      while (times[firstValidRateIndex] == 0)
      {
         firstValidRateIndex++;
         if (firstValidRateIndex >= seriesPointer)
            break;
      }
      for (int j = 0; j < calendarEventsTabSize; j++)
      {
         if (calEvStartInd == -1)
         {
            if (calendarEvents[j].eventDateTime >= times[firstValidRateIndex])
               calEvStartInd = j;
         }

         if (calEvEndInd == -1)
         {
            if (calendarEvents[calendarEventsTabSize - 1 - j].eventDateTime <= times[seriesPointer])
               calEvEndInd = calendarEventsTabSize - 1 - j;
         }
      }
   }

   int transactionPStartInd = -1;
   int transactionPEndInd = -1;

   if (isOrdersPoints)
   {
      int firstValidRateIndex = i;
      while (times[firstValidRateIndex] == 0)
      {
         firstValidRateIndex++;
         if (firstValidRateIndex >= seriesPointer)
            break;
      }
      for (int j = 0; j < transactionsTabSize; j++)
      {
         if (transactionPStartInd == -1)
         {
            if (transactionsTab[j][0] >= times[firstValidRateIndex])
               transactionPStartInd = j;
         }

         if (transactionPEndInd == -1)
         {
            if (transactionsTab[transactionsTabSize - 1 - j][0] <= times[seriesPointer])
               transactionPEndInd = transactionsTabSize - 1 - j;
         }
      }
   }

   bool once = false;

   rct.left = m_data_area.left;
   uint intevalSepVLineOffst = (barChartBarWidth & 4) != 0;
   unsigned long long interval_factor = intervals_quot[interval_idx];
   int counter = 0;

   char timeSepSignature[6];
   timeSepSignature[2] = ':';
   timeSepSignature[5] = '\0';
   ulong dayHourSeconds1 = 0;
   ulong dayHourSeconds2 = 0;
   ulong hour1;

   for (; i <= seriesPointer; i++, x += dx)
   {
      double oPr = openPrices[i];
      double cPr = closePrices[i];
      double high = highPrices[i];
      double low = lowPrices[i];

      if (oPr == 0 || cPr == 0 || high == 0 || low == 0)
         continue;

      if (isCalendarEvents & showCalendarEvents)
      {
         if (((calEvStartInd & 0x80000000) | (calEvEndInd & 0x80000000)) == 0)
         {
            int calendarSignOffset = 20;
            int gross = 0;
            while (calEvStartInd <= calEvEndInd)
            {
               if (times[i] >= calendarEvents[calEvStartInd].eventDateTime)
               {
                  if (calendarEvents[calEvStartInd].importance == 2)
                     gross = 0;
                  else if (calendarEvents[calEvStartInd].importance == 3)
                     gross = 3;
                  if (calendarEvents[calEvStartInd].eventChange > 0)
                  {
                     FillTriangle(x - 4 - gross, calendarSignOffset + 10 + gross, x + 6 + gross, calendarSignOffset + 10 + gross, x + 1, calendarSignOffset - gross, orderPointBuyColor);
                  }
                  else if (calendarEvents[calEvStartInd].eventChange < 0)
                  {
                     FillTriangle(x - 4 - gross, calendarSignOffset - gross, x + 6 + gross, calendarSignOffset - gross, x + 1, calendarSignOffset + 10 + gross, orderPointSellColor);
                  }
                  else
                  {
                     FillCircle(x + 1, calendarSignOffset + 5, 5 + gross, calendarEventCircleColor);
                  }

                  INFOSTRING[0] = WNUMBERS_A[calendarEvents[calEvStartInd].importance][0];
                  INFOSTRING[1] = ',';
                  int idx = 2;

                  int ptr = 0;
                  while (calendarEvents[calEvStartInd].eventSymbol[ptr] != '\0')
                  {
                     INFOSTRING[idx] = calendarEvents[calEvStartInd].eventSymbol[ptr];
                     idx++;
                     ptr++;
                  }
                  INFOSTRING[idx] = ',';
                  idx++;

                  ptr = 0;
                  while (calendarEvents[calEvStartInd].eventDescription[ptr] != '\0')
                  {
                     INFOSTRING[idx] = calendarEvents[calEvStartInd].eventDescription[ptr];
                     idx++;
                     ptr++;
                  }

                  if (rct.top > 1 && rct.top < m_height - 9)
                     DrawBitTimeSepStamp_09(INFOSTRING, idx, x - 9 - idx * 5, calendarSignOffset + 5 - 4, m_data_area.left, m_data_area.right);

                  calEvStartInd++;
                  calendarSignOffset += 20;
               }
               else
               {
                  break;
               }
            }
         }
      }

      if (isOrdersPoints)
      {
         // Je�li logiczna suma bit�w znak�w == 0 :obie liczby s� nieujemne
         if (((transactionPStartInd & 0x80000000) | (transactionPEndInd & 0x80000000)) == 0)
         {
            int transactionPointOffset = -100;
            int transactionPointOffsetMultiplier = -1;
            bool initializing = true;
            while (transactionPStartInd <= transactionPEndInd)
            {
               if (times[i] >= transactionsTab[transactionPStartInd][0])
               {
                  double pointPrice = transactionsTab[transactionPStartInd][2] * _Point;
                  pointPrice -= y_scale_shift;

                  int baseAnchorPt = (int)(m_y_0 - pointPrice * m_scale_y + 0.5);
                  if (initializing)
                  {
                     if (baseAnchorPt < (m_data_area.bottom - m_data_area.top) / 2)
                     {
                        transactionPointOffsetMultiplier = 1;
                        transactionPointOffset = 100;
                     }
                     initializing = false;
                  }

                  int _pt = x - 11;
                  uint _clr = orderPointBuyColor;
                  if (transactionsTab[transactionPStartInd][1] > 1)
                     _pt = x + 13;
                  if (transactionsTab[transactionPStartInd][1] & 1) // If odd number
                     _clr = orderPointSellColor;
                  FillTriangle(_pt, baseAnchorPt + 8, _pt, baseAnchorPt - 8, x + 1, baseAnchorPt, _clr);

                  LineVertical(x + 1, baseAnchorPt + transactionPointOffset, baseAnchorPt, orderPointLineColor);

                  rct.top = baseAnchorPt + transactionPointOffset - 8;
                  if (rct.top > 1 && rct.top < m_height - 9)
                     DrawBitTimeSepStamp_09(transactionsDescriptions[transactionPStartInd], (int)transactionsTab[transactionPStartInd][3], x - (((int)transactionsTab[transactionPStartInd][3] * 8) / 2), rct.top, m_data_area.left, m_data_area.right);

                  transactionPStartInd++;
                  transactionPointOffset += 10 * transactionPointOffsetMultiplier;
               }
               else
               {
                  break;
               }
            }
         }
      }

      if (drawVerticalGrid)
      {
         if (counter % 10 == 0)
            LineVerticalDott(x + 1, 10, m_data_area.bottom, verticalGridColor);
      }

      if (i > 0)
      {
         if (interval > 0)
         {
            if (interval == 60)
            {
               time_t _tm = times[i];
               ulong secs = _tm - dayHourSeconds1;
               if (secs >= 24 * 60 * 60)
               {
                  secs = _tm;
                  ulong days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
                  secs -= days * 24 * 60 * 60;
                  dayHourSeconds1 = _tm - secs;
               }
               hour1 = (secs * 1193047) >> 32;

               _tm = times[i - 1];
               secs = _tm - dayHourSeconds2;
               if (secs >= 24 * 60 * 60)
               {
                  secs = _tm;
                  ulong days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
                  secs -= days * 24 * 60 * 60;
                  dayHourSeconds2 = _tm - secs;
               }
               ulong _hr = (secs * 1193047) >> 32;
               if (hour1 != _hr)
               {
                  SafeSortedLineVertical(x + intevalSepVLineOffst, 10, m_data_area.bottom, timeIntervalObjColor);

                  timeSepSignature[0] = TIMETABLE_A[hour1][0];
                  timeSepSignature[1] = TIMETABLE_A[hour1][1];
                  timeSepSignature[3] = '0';
                  timeSepSignature[4] = '0';

                  DrawBitTimeSepStamp_09(timeSepSignature, 5, x - 11, 2, m_data_area.left + 1, m_data_area.right - 1);
               }
            }
            else
            {
               ulong intervalDivider = interval * 60;

               time_t _tm = times[i];
               ulong secs = _tm - dayHourSeconds1;
               if (secs >= 3600)
               {
                  secs = _tm;
                  uint days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
                  secs -= days * 24 * 60 * 60;
                  hour1 = (secs * 1193047) >> 32;
                  secs -= hour1 * 3600;
                  dayHourSeconds1 = _tm - secs;
               }
               ulong actualHourFragment = (secs * interval_factor) >> 32;

               _tm = times[i - 1];
               secs = _tm - dayHourSeconds2;
               if (secs >= 3600)
               {
                  secs = _tm;
                  uint days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
                  secs -= days * 24 * 60 * 60;
                  ulong hr2 = (secs * 1193047) >> 32;
                  secs -= hr2 * 3600;
                  dayHourSeconds2 = _tm - secs;
               }
               ulong previousHourFragment = (secs * interval_factor) >> 32;

               if (actualHourFragment != previousHourFragment)
               {
                  SafeSortedLineVertical(x + intevalSepVLineOffst, 10, m_data_area.bottom, timeIntervalObjColor);

                  ulong mn = actualHourFragment * interval;

                  timeSepSignature[0] = TIMETABLE_A[hour1][0];
                  timeSepSignature[1] = TIMETABLE_A[hour1][1];
                  timeSepSignature[3] = TIMETABLE_A[mn][0];
                  timeSepSignature[4] = TIMETABLE_A[mn][1];

                  DrawBitTimeSepStamp_09(timeSepSignature, 5, x - 11, 2, m_data_area.left + 1, m_data_area.right - 1);
               }
            }
         }
      }

      if (mTimeParameterVis)
      {
         if (timeParameters[i] > 0)
         {
            int colorIdx = 0; // grey
            if (mColorTimeParameter)
               colorIdx = 1; // color

            ulong timeParamDiff = timeParameters[i] - tmParamMin + 1;
            ulong _tmp_value = ((timeParamDiff * tmParamFactor) >> 21);
            if (_tmp_value > 255)
               _tmp_value = 255;
            ulong tmParamIdx = 255 - _tmp_value;

            if (barChartBarWidth == 4)
               SafeSortedFillRectangle(x, (m_data_area.bottom - tmParamFigureHight), x + 2, m_data_area.bottom, timeParamColors[tmParamIdx][colorIdx]);
            else
               SafeSortedLineVertical(x, (m_data_area.bottom - tmParamFigureHight), m_data_area.bottom, timeParamColors[tmParamIdx][colorIdx]);
         }
      }

      if (mainPlotVis)
      {
         if (barChartBarWidth == 4)
         {
            bool direction = oPr < cPr;
            oPr -= y_scale_shift;
            cPr -= y_scale_shift;
            high -= y_scale_shift;
            low -= y_scale_shift;

            int openPr = (int)(m_y_0 - oPr * m_scale_y + 0.5);
            int closePr = (int)(m_y_0 - cPr * m_scale_y + 0.5);
            int highPr = (int)(m_y_0 - high * m_scale_y + 0.5);
            int lowPr = (int)(m_y_0 - low * m_scale_y + 0.5);

            if (direction)
            {
               SafeSortedLineVertical(x + 1, highPr, lowPr, bearCandleColor);
               SafeSortedLineVertical(x + 1, closePr, openPr, bullCandleColor);
               SafeSortedRectangle(x, closePr, x + 2, openPr, bearCandleColor);
            }
            else
            {
               SafeSortedLineVertical(x + 1, highPr, lowPr, bearCandleColor);
               SafeSortedFillRectangle(x, openPr, x + 2, closePr, bearCandleColor);
            }
         }
         else
         {
            high -= y_scale_shift;
            low -= y_scale_shift;
            int highPr = (int)(m_y_0 - high * m_scale_y + 0.5);
            int lowPr = (int)(m_y_0 - low * m_scale_y + 0.5);
            SafeSortedLineVertical(x, highPr, lowPr, bearCandleColor);
         }
      }
      counter++;
   }

   // draw current bar on chart
   if (actualBidLow != DBL_MAX)
   {
      double _prHigh = actualBidHigh;
      double _prLow = actualBidLow;
      if (m_v_scale_min > 0)
      {
         _prHigh -= m_v_scale_min;
         _prLow -= m_v_scale_min;
      }
      int _prHighPt = (int)(m_y_0 - _prHigh * m_scale_y + 0.5);
      int _prLowPt = (int)(m_y_0 - _prLow * m_scale_y + 0.5);
      LineVertical(x, _prHighPt, _prLowPt, bearCandleColor);
   }
   // draw current price level sign on chart
   if (actualBidPrice != 0)
   {
      double _prLvl = actualBidPrice;
      if (m_v_scale_min > 0)
      {
         _prLvl -= m_v_scale_min;
      }
      int _prLvlPt = (int)(m_y_0 - _prLvl * m_scale_y + 0.5);
      LineHorizontal(x, x + 6, _prLvlPt, bearCandleColor);
   }

   vScaleParamsChanged = false;
}
void CBarChart::DrawSignedLevels()
{
   RECT rct;
   rct.left = m_data_area.left + 1;
   double vvvOffst = 0;
   if (m_v_scale_min > 0)
      vvvOffst = m_v_scale_min;

   for (int j = 0; j < signedLevelsArraySize; j++)
   {
      if (signedLevels[j] < m_v_scale_min)
         continue;
      else if (signedLevels[j] > m_v_scale_max)
         continue;

      double y_raw = signedLevels[j];
      y_raw -= vvvOffst;
      int y_val = (int)(m_y_0 - y_raw * m_scale_y + 0.5);

      SafeSortedLineHorizontalDott(m_data_area.left + 1, m_data_area.right - 1, y_val, signedLevelsObjColor);
      if (isSignedLevelsDescriptions)
      {
         rct.top = y_val - 11;
         if (rct.top > 1 && rct.top < m_height - 12)
            DrawBitText_12(signedLevelsDescriptions[j], 42, m_data_area.left + 1, rct.top, true, m_data_area.left + 1, m_data_area.right - 1);
      }
      FillTriangle(m_data_area.right - 1, y_val + 4, m_data_area.right - 1, y_val - 4, m_data_area.right - 10, y_val, signedLevelsObjColor);
   }
}
void CBarChart::DrawMProfile(int dataStartIndex)
{
   if (mProfileSize > 1)
   {
      int rangeHeight = m_y_min + 1;
      ulong dyRaw = ((m_y_min - m_y_max) * mProfileSizeFactor) >> 31;
      uint dy = (uint)dyRaw;

      bool multiplierValid = false;
      double multiplier = 1;

      double vvvOffst = 0;
      if (m_v_scale_min > 0)
         vvvOffst = m_v_scale_min;

      if (dy == 0)
      {
         if (rangeHeight != rescaledMProfileTabSize)
         {
            delete[] rescaledMProfile;
            rescaledMProfile = new double[rangeHeight];
            rescaledMProfileTabSize = rangeHeight;
         }

         memset(rescaledMProfile, 0, rangeHeight * sizeof(double));

         double mostSignificantPricePart = int((openPrices[dataStartIndex] * _DigitsMultiplier) / 100000) * POWER_OF_10[5 - _Digits];
         double vvv = NormalizeDouble(mProfileStartIndex * _Point + mostSignificantPricePart, _Digits);

         vvv -= vvvOffst;
         int yInd = (int)(m_y_0 - vvv * m_scale_y + 0.5);

         if ((uint)yInd < (uint)rangeHeight)
         {

            rescaledMProfile[yInd] += mProfileData[mProfileStartIndex];
         }

         for (int i = mProfileStartIndex + 1; i < mProfileSize + mProfileStartIndex; i++)
         {
            vvv = i * _Point + mostSignificantPricePart;
            vvv -= vvvOffst;
            yInd = (int)(m_y_0 - vvv * m_scale_y + 0.5);

            if ((uint)yInd < (uint)rangeHeight)
            {
               if (mProfileData[i] > 0)
                  rescaledMProfile[yInd] += mProfileData[i];
            }
         }

         double maxVal = MaxInArray2(rescaledMProfile, 0, rangeHeight);

         if (maxVal != 0)
         {
            multiplierValid = true;
            multiplier = 1.0 / maxVal;
         }
      }
      else
      {
         double maxVal = MaxInArray2(mProfileData, mProfileStartIndex, mProfileSize);

         multiplierValid = false;

         if (maxVal != 0)
         {
            multiplierValid = true;
            multiplier = 1.0 / maxVal;
         }
      }

      if (multiplierValid)
      {
         if (dy == 0)
         {
            int new_x = 0;
            for (int i = m_y_max; i < rangeHeight; i++)
            {
               if (rescaledMProfile[i] > 0)
               {
                  new_x = (m_data_area.left + (int)(((double)rescaledMProfile[i] * multiplier) * 200.0 + 0.5));
                  SafeSortedLineHorizontal(m_data_area.left + 1, new_x, i, mProfileColor);
               }
            }
         }
         else
         {
            double mostSignificantPricePart = int((openPrices[dataStartIndex] * _DigitsMultiplier) / 100000) * POWER_OF_10[5 - _Digits];
            double vvv = mProfileStartIndex * _Point + mostSignificantPricePart;

            vvv -= vvvOffst;
            int yyy2 = (int)(m_y_0 - vvv * m_scale_y + 0.5);

            int new_x = 0;

            if (mProfileData[mProfileStartIndex] > 0)
            {
               new_x = (m_data_area.left + (int)(((double)mProfileData[mProfileStartIndex] * multiplier) * 200.0 + 0.5));
               SafeSortedFillRectangle(m_data_area.left + 1, yyy2 + 1 - dy, new_x, yyy2, mProfileColor);
            }

            for (int i = mProfileStartIndex + 1; i < mProfileSize + mProfileStartIndex; i++)
            {
               int yyy1 = yyy2;
               vvv = i * _Point + mostSignificantPricePart;
               vvv -= vvvOffst;

               yyy2 = (int)(m_y_0 - vvv * m_scale_y + 0.5);

               int finalY1 = yyy1 - dy;
               int finalY2 = yyy2 + 1 - dy;
               if (finalY1 > finalY2)
               {
                  finalY1 += finalY2;
                  finalY2 = finalY1 - finalY2;
                  finalY1 -= finalY2;
               }
               if (mProfileData[i] > 0)
               {
                  new_x = (m_data_area.left + (int)(((double)mProfileData[i] * multiplier) * 200.0 + 0.5));
                  SafeSortedFillRectangle(m_data_area.left + 1, finalY1, new_x, finalY2, mProfileColor);
               }
            }
         }
      }
   }
}