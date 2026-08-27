#include "StdAfx.h"
#include "TickChart.h"

CTickChart::CTickChart(void)
{
   ShowFlags(FLAG_SHOW_LEGEND | FLAGS_SHOW_SCALES | FLAG_SHOW_GRID);

   DarkMode = true;

   mDataAreaStartPoint = 0;
   initialized = false;
   boldenChart = false;
   tickSampleWidth = 4;
   mainPlotVis = true;
   barsDataMultiplier = 1;
   interval = 5;
   mProfileStartIndex = -1;
   mProfileSize = 0;
   mProfileSizeFactor = 1;
   totalMProfileSize = 100000;
   mColorTimeParameter = true;
   calendarEventsTabSize = 0;
   transactionsTabSize = 0;

   mProfileDataBidVis = false;
   mProfileDataAskVis = false;

   chartVisSize = 2;

   seriesSize = 10000;
   chartWidthInSamples = 2000;
   seriesPointer = chartWidthInSamples - 2;

   drawVerticalGrid = false;

   times = new time_t[seriesSize];
   memset(times, 0, seriesSize * sizeof(time_t));

   askPrices = new double[seriesSize];
   memset(askPrices, 0, seriesSize * sizeof(double));

   bidPrices = new double[seriesSize];
   memset(bidPrices, 0, seriesSize * sizeof(double));

   timeParameters = new ulong[seriesSize];
   memset(timeParameters, 0, seriesSize * sizeof(uint));

   mCumulativeCountDataBid = new int[seriesSize];
   memset(mCumulativeCountDataBid, 0, seriesSize * sizeof(int));

   mCumulativeCountDataAsk = new int[seriesSize];
   memset(mCumulativeCountDataAsk, 0, seriesSize * sizeof(int));

   ticksArrived = new short[seriesSize];
   memset(ticksArrived, 0, seriesSize * sizeof(short));

   travelledDistance = new short[seriesSize];
   memset(travelledDistance, 0, seriesSize * sizeof(short));

   travelledRoad = new short[seriesSize];
   memset(travelledRoad, 0, seriesSize * sizeof(short));

   mProfileDataBid = new int[totalMProfileSize];
   memset(mProfileDataBid, 0, seriesSize * sizeof(int));

   mProfileDataAsk = new int[totalMProfileSize];
   memset(mProfileDataAsk, 0, seriesSize * sizeof(int));

   chartVisibility = new int[chartVisSize];
   chartVisibility[0] = 1;
   chartVisibility[1] = 1;

   isCalendarEvents = false;
   showCalendarEvents = false;
   isSignedLevelsDescriptions = false;
   isOrdersPoints = false;

   travelledDistanceVis = false;
   travelledRoadVis = false;
   ticksArrivedVis = false;

   travelledDistanceScaleDrawn = false;
   travelledRoadScaleDrawn = false;
   ticksArrivedScaleDrawn = false;

   pipsDivider = 1;

   transactionsTab = nullptr;
   signedLevelsArraySize = 0;
   calendarEvents = nullptr;
   signedLevelsDescriptions = nullptr;
   signedLevels = nullptr;

   decimalSep = 40;

   TIMESTAMP_SHORT[2] = TIMESTAMP_SHORT[5] = ':';
   TIMESTAMP_SHORT[8] = '\0';

   erase_bkg_hor = nullptr;
   erase_bkg_ver = nullptr;

   SetColorMode(DarkMode);

   for (int i = 0; i < 256; i++)
   {
      timeParamColors[i][0] = (uint)XRGB_gdi(255 - i, 255 - i, 255 - i);
      timeParamColors[i][1] = (uint)XRGB_gdi(255 - i, 0, 0);
   }

   interval_idx = 1; // for 5min
   intervals_quot[0] = 4294967296;
   intervals_quot[1] = 858993460; //(unsigned long long)ceil(4294967296/5.0);
   intervals_quot[2] = 286331154; //(unsigned long long)ceil(4294967296/15.0);
   intervals_quot[3] = 143165577; //(unsigned long long)ceil(4294967296/30.0);
   intervals_quot[4] = 71582789;  //(unsigned long long)ceil(4294967296/60.0);

   erase_flags = 0;
   _extremumCount = 0;

   transactionsDescriptions = nullptr;

   rescaledMProfileTabSize = 1;
   rescaledMProfileAsk = new double[rescaledMProfileTabSize];
   rescaledMProfileBid = new double[rescaledMProfileTabSize];
}

CTickChart::~CTickChart(void)
{
   delete[] chartVisibility;
   delete[] askPrices;
   delete[] bidPrices;
   delete[] timeParameters;
   delete[] mCumulativeCountDataBid;
   delete[] mCumulativeCountDataAsk;
   delete[] travelledDistance;
   delete[] travelledRoad;
   delete[] ticksArrived;
   delete[] mProfileDataBid;
   delete[] mProfileDataAsk;
   delete[] times;
   if (transactionsTab != nullptr)
      delete[] transactionsTab;
   if (calendarEvents != nullptr)
      delete[] calendarEvents;
   if (signedLevelsDescriptions != nullptr)
      delete[] signedLevelsDescriptions;
   if (signedLevels != nullptr)
      delete[] signedLevels;

   if (erase_bkg_hor != nullptr)
      delete[] erase_bkg_hor;
   if (erase_bkg_ver != nullptr)
      delete[] erase_bkg_ver;

   if (transactionsDescriptions != nullptr)
      delete[] transactionsDescriptions;

   delete[] rescaledMProfileAsk;
   delete[] rescaledMProfileBid;
}

bool CTickChart::Create(HWND hWnd, const int width, const int height, double pointValue, int digits)
{
   if (!ChartCanvas::Create(hWnd, width, height))
      return (false);

   if (erase_bkg_hor != nullptr)
      delete[] erase_bkg_hor;
   if (erase_bkg_ver != nullptr)
      delete[] erase_bkg_ver;

   erase_bkg_hor = new int[height];
   erase_bkg_ver = new int[width];

   memset(erase_bkg_hor, 0, m_height * 4);
   memset(erase_bkg_ver, 0, m_width * 4);

   _Digits = digits;
   _Point = pointValue;
   _DigitsMultiplier = POWER_OF_10[digits];

   return (true);
}
void CTickChart::SetColorMode(const bool value)
{
   DarkMode = value;
   if (DarkMode)
   {
      signedLevelsObjColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(80, 80, 80);
      signedLevelsTextColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(150, 150, 150);
      mProfileBidColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(196, 196, 255);
      mProfileAskColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(255, 196, 196);
      calendarEventCircleColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(128, 128, 128);
      orderPointBuyColor = (uint)XRGB_gdi(51, 51, 226);
      orderPointSellColor = (uint)XRGB_gdi(226, 51, 51);
      orderPointCloseColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(218, 165, 32);
      orderPointLineColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(212, 212, 212);
      orderPointTextColor = (uint)XRGB_gdi(255, 255, 255);
      travelledDistanceHLineColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(255, 220, 255);
      travelledDistanceColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(255, 150, 255);
      travelledRoadColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(100, 100, 100);
      ticksArrivedColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(100, 232, 100);
      askLineColor = (uint)XRGB_gdi(255, 150, 150);
      bidLineColor = (uint)XRGB_gdi(150, 150, 255);
      cumulativeBidDataColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(200, 200, 200);
      cumulativeAskDataColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(200, 80, 50);
      timeParameterObjColorNeutral = (uint)XRGB_gdi(111, 111, 0);
      timeParameterObjColorBid = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(0, 0, 255);
      timeParameterObjColorAsk = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(220, 0, 0);
      verticalGridColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(230, 230, 230);
      duplicatedScaleColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(128, 128, 128);
      timeIntervalTextColor = (uint)XRGB_gdi(255, 255, 255);
      timeIntervalObjColor = (uint)XRGB_gdi(255, 255, 255) - (uint)XRGB_gdi(212, 212, 212);
   }
   else
   {
      signedLevelsObjColor = (uint)XRGB_gdi(80, 80, 80);
      signedLevelsTextColor = (uint)XRGB_gdi(150, 150, 150);
      mProfileBidColor = (uint)XRGB_gdi(196, 196, 255);
      mProfileAskColor = (uint)XRGB_gdi(255, 196, 196);
      calendarEventCircleColor = (uint)XRGB_gdi(128, 128, 128);
      orderPointBuyColor = (uint)XRGB_gdi(0, 0, 255);
      orderPointSellColor = (uint)XRGB_gdi(255, 0, 0);
      orderPointCloseColor = (uint)XRGB_gdi(218, 165, 32);
      orderPointLineColor = (uint)XRGB_gdi(212, 212, 212);
      orderPointTextColor = (uint)XRGB_gdi(0, 0, 0);
      travelledDistanceHLineColor = (uint)XRGB_gdi(255, 220, 255);
      travelledDistanceColor = (uint)XRGB_gdi(255, 150, 255);
      travelledRoadColor = (uint)XRGB_gdi(100, 100, 100);
      ticksArrivedColor = (uint)XRGB_gdi(100, 232, 100);
      askLineColor = (uint)XRGB_gdi(220, 0, 0);
      bidLineColor = (uint)XRGB_gdi(0, 0, 255);
      cumulativeBidDataColor = (uint)XRGB_gdi(200, 200, 200);
      cumulativeAskDataColor = (uint)XRGB_gdi(200, 80, 50);
      timeParameterObjColorNeutral = (uint)XRGB_gdi(62, 225, 62);
      timeParameterObjColorBid = (uint)XRGB_gdi(0, 0, 255);
      timeParameterObjColorAsk = (uint)XRGB_gdi(220, 0, 0);
      verticalGridColor = (uint)XRGB_gdi(230, 230, 230);
      duplicatedScaleColor = (uint)XRGB_gdi(128, 128, 128);
      timeIntervalTextColor = (uint)XRGB_gdi(0, 0, 0);
      timeIntervalObjColor = (uint)XRGB_gdi(212, 212, 212);
   }
}
void CTickChart::DrawBackground(void)
{
   if (vScaleParamsChanged)
   {
      memset(m_pixels, m_color_background, m_width * m_height * 4);
      Rectangle(0, 0, m_width - 1, m_height - 1, m_color_border);
   }
   else if (erase_flags)
   {
      // If Market Profile, etc.
      int len = (m_data_area.right - m_data_area.left) - 10;
      uint *offst = &m_pixels[m_data_area.left + 1];
      for (int i = m_height - 2; i--;)
      {
         offst += m_width;
         memset(offst, m_color_background, len * 4);
      }
   }
   else
   {
      int len = (m_data_area.right - m_data_area.left) - 10;
      uint *offst = &m_pixels[m_data_area.left + 1];
      int size = len * 4;

      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      for (int *i = &erase_bkg_hor[8]; i < erase_bkg_hor + m_height - 1; i++)
      {
         offst += m_width;
         if (*i) // if *i!=0
            memset(offst, m_color_background, size);
      }

      // Clearing the Clock
      size = 224; // 56*4 : 7px * 8letters * 4bytes
      offst = &m_pixels[m_data_area.left + 6];
      offst += 26 * m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);

      offst += 4 * m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);
      offst += m_width;
      memset(offst, m_color_background, size);

      // Lines vertical
      for (int i = 1; i < m_width - 1; i++)
      {
         if (erase_bkg_ver[i] != 0)
         {
            int index = m_width + i;
            for (int j = 1; j < m_height - 1; j++, index += m_width)
            {
               m_pixels[index] = m_color_background;
            }
         }
      }
   }
   memset(erase_bkg_hor, 0, m_height * 4);
   memset(erase_bkg_ver, 0, m_width * 4);
}
void CTickChart::FillSeries(const double askPrice, const double bidPrice, const time_t time, const uint timeParameter, const int cumulativeDataAsk, const int cumulativeDataBid)
{
   for (int i = 0; i < seriesSize; i++)
   {
      askPrices[i] = askPrice;
      bidPrices[i] = bidPrice;
      mCumulativeCountDataAsk[i] = cumulativeDataAsk;
      mCumulativeCountDataBid[i] = cumulativeDataBid;
      times[i] = time;
      timeParameters[i] = timeParameter;
      travelledDistance[i] = 0;
      travelledRoad[i] = 0;
      ticksArrived[i] = 0;
   }

   long long *tab1 = (long long *)(mProfileDataBid);
   long long *tab2 = (long long *)(mProfileDataAsk);

   for (int i = 0; i < totalMProfileSize / 2; i++)
   {
      tab1[i] = 0;
      tab2[i] = 0;
   }

   seriesPointer = chartWidthInSamples - 2;
   initialized = false;
}
void CTickChart::AppendPricesTimeAndParameters(const double askPrice, const double bidPrice, const time_t time, const ulong timeParameter, const int askChange, const int bidChange, const short road, const short distance, const short ticks)
{
   seriesPointer++;
   if (seriesPointer == seriesSize)
   {
      memmove(askPrices, &askPrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(bidPrices, &bidPrices[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(double));
      memmove(times, &times[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(time_t));
      memmove(timeParameters, &timeParameters[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(ulong));
      memmove(mCumulativeCountDataAsk, &mCumulativeCountDataAsk[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(int));
      memmove(mCumulativeCountDataBid, &mCumulativeCountDataBid[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(int));
      memmove(travelledDistance, &travelledDistance[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(short));
      memmove(travelledRoad, &travelledRoad[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(short));
      memmove(ticksArrived, &ticksArrived[(seriesSize - chartWidthInSamples)], chartWidthInSamples * sizeof(short));

      seriesPointer = chartWidthInSamples;
   }

   askPrices[seriesPointer] = askPrice;
   bidPrices[seriesPointer] = bidPrice;
   times[seriesPointer] = time;

   // Function ( 1 / x ) * 10241000UL
   //+10 to flatten function 1 / x, for x near 1
   timeParameters[seriesPointer] = ((1024UL * 1000UL) / (timeParameter + 10)) + 1;

   travelledDistance[seriesPointer] = distance;
   travelledRoad[seriesPointer] = road;
   ticksArrived[seriesPointer] = ticks;

   if (bidChange == 0)
      mCumulativeCountDataBid[seriesPointer] = mCumulativeCountDataBid[seriesPointer - 1];
   else
   {
      if ((bidChange & 0x80000000) == (mCumulativeCountDataBid[seriesPointer - 1] & 0x80000000))
         mCumulativeCountDataBid[seriesPointer] = mCumulativeCountDataBid[seriesPointer - 1] + bidChange;
      else
         mCumulativeCountDataBid[seriesPointer] = bidChange;
   }

   if (askChange == 0)
      mCumulativeCountDataAsk[seriesPointer] = mCumulativeCountDataAsk[seriesPointer - 1];
   else
   {
      if ((askChange & 0x80000000) == (mCumulativeCountDataAsk[seriesPointer - 1] & 0x80000000))
         mCumulativeCountDataAsk[seriesPointer] = mCumulativeCountDataAsk[seriesPointer - 1] + askChange;
      else
         mCumulativeCountDataAsk[seriesPointer] = askChange;
   }

   initialized = true;
}

void CTickChart::UpdateMarketProfile(const double askPrice, const double bidPrice)
{
   int bidInd = (int)(bidPrice * _DigitsMultiplier + 0.5) % 100000;
   int askInd = (int)(askPrice * _DigitsMultiplier + 0.5) % 100000;

   mProfileDataBid[bidInd]++;
   mProfileDataAsk[askInd]++;
}

void CTickChart::MoveMarketProfileRange(const double lowRange, const double highRange)
{
   mProfileStartIndex = ((int)(lowRange * _DigitsMultiplier + 0.5) % 100000);
   int endInd = ((int)(highRange * _DigitsMultiplier + 0.5) % 100000);

   mProfileSize = endInd - mProfileStartIndex;
   if (mProfileSize > 1)
      mProfileSizeFactor = 2147483648 / (mProfileSize - 1);
   else
      mProfileSizeFactor = 2147483648 / (mProfileSize);
}
void CTickChart::AppendSignedLevels(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update)
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
      if (signedLevelsDescriptions != nullptr)
      {
         delete[] signedLevelsDescriptions;
         signedLevelsDescriptions = nullptr;
      }
      if (signedLevels != nullptr)
      {
         delete[] signedLevels;
         signedLevels = nullptr;
      }
      signedLevelsArraySize = 0;
   }
   if (update)
   {
      vScaleParamsChanged = true;
      Redraw();
   }
}
void CTickChart::AppendCalendarEvents(CalendarEvent clEvents[], int tabSize)
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
}
void CTickChart::AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size)
{
   if (size > 0)
   {
      if (size != transactionsTabSize)
      {
         delete[] transactionsTab;
         transactionsTab = new long long[size][4];

         if (transactionsDescriptions != nullptr)
            delete[] transactionsDescriptions;

         transactionsDescriptions = new char[size][64];
      }
      memcpy(transactionsTab, transactions, size * 4 * sizeof(long long));
      memcpy(transactionsDescriptions, descriptions, size * 64 * sizeof(char));
      transactionsTabSize = size;
      isOrdersPoints = true;
      erase_flags |= IS_SHOW_TRANSACTIONS;
   }
   else
   {
      isOrdersPoints = false;
      erase_flags &= ~IS_SHOW_TRANSACTIONS;
      transactionsTabSize = 0;
   }
   vScaleParamsChanged = true;
}
void CTickChart::ChartVisibility(const uint pos, const int value, const bool update)
{
   if (pos < chartVisSize)
   {
      chartVisibility[pos] = value;
      if (update)
      {
         Redraw();
      }
   }
}
void CTickChart::RefreshWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   Update(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void CTickChart::RefreshWindow()
{
   Update();
}
void CTickChart::UpdateChart(bool vScaleParChanged)
{
   if (initialized)
   {
      if (vScaleParChanged)
         vScaleParamsChanged = vScaleParChanged;

      Redraw();
   }
}
void CTickChart::UpdateChart(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc, bool vScaleParChanged)
{
   if (initialized)
   {
      if (vScaleParChanged)
         vScaleParamsChanged = vScaleParChanged;
      Redraw(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
   }
}
void CTickChart::DrawChart(void)
{
   if (initialized)
   {
      DrawData(0);
   }
}
void CTickChart::DrawData(const uint index)
{
   mDataAreaStartPoint = m_data_area.left;
   int dx = 1 + (tickSampleWidth >> 2);

   uint extremumCount = _extremumCount;
   int extremumStartIndex = seriesPointer - extremumCount + 1;

   if (mProfileDataAskVis | mProfileDataBidVis)
   {
      DrawMProfile(extremumStartIndex);
   }
   if (mSignedLevelsVis && signedLevelsArraySize)
   {
      DrawSignedLevels();
   }

   int i = extremumStartIndex + 1;

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

   int idxIncr = 1;
   int counter = 1;
   int rectDt1 = 0, rectDt2 = 0;
   uint tmParamClr;
   uint *tmParamPaddingClr;
   if (tickSampleWidth == 4)
      tmParamPaddingClr = &tmParamClr;
   else
   {
      tmParamPaddingClr = &m_color_background;

      if (tickSampleWidth == 1)
         idxIncr = 2;
   }

   int x = m_data_area.left + 1;
   int y1_ask = 0;
   int y1_bid = 0;
   int y2_ask = 0;
   int y2_bid = 0;
   double val_ask = 0;
   double val_bid = 0;
   double y_scale_shift = 0;

   int minY = 1, maxY = m_height - 2;
   uint valid_range = maxY - minY - 1;

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

   val_ask = askPrices[extremumStartIndex];
   val_bid = bidPrices[extremumStartIndex];

   if (m_v_scale_min > 0)
   {
      y_scale_shift = m_v_scale_min;
      val_ask -= m_v_scale_min;
      val_bid -= m_v_scale_min;
   }

   y2_ask = (int)(m_y_0 - val_ask * m_scale_y + 0.5);
   y2_bid = (int)(m_y_0 - val_bid * m_scale_y + 0.5);

   // Assuming that the bid will not be higher than the ask
   // Note: chart coordinates on the Y-axis increase downward.

   y2_ask = std::max(y2_ask, minY);
   y2_ask = std::min(y2_ask, maxY);
   y2_bid = std::max(y2_bid, minY);
   y2_bid = std::min(y2_bid, maxY);

   int lastBid[2] = {-1, -1}; // 0-top,1-bottom, top<bottom
   int lastAsk[2] = {-1, -1};
   int currentBid[2] = {-1, -1};
   int currentAsk[2] = {-1, -1};

   RECT rct;
   rct.left = m_data_area.left;
   int maxPixelRealPrice = y2_bid;
   int minPixelRealPrice = y2_ask;
   unsigned long long interval_factor = intervals_quot[interval_idx];

   uint tmParamAnchorPt;
   char timeSepSignature[6];
   timeSepSignature[2] = ':';
   timeSepSignature[5] = '\0';
   ulong dayHourSeconds1 = 0;
   ulong dayHourSeconds2 = 0;
   ulong hour1;

   for (; i <= seriesPointer; i += idxIncr, x += dx)
   {
      if (tickSampleWidth == 1)
      {
         val_ask = askPrices[i - 1];
         val_bid = bidPrices[i - 1];

         val_ask -= y_scale_shift;
         val_bid -= y_scale_shift;

         y1_ask = (int)(m_y_0 - val_ask * m_scale_y + 0.5);
         y1_bid = (int)(m_y_0 - val_bid * m_scale_y + 0.5);

         y1_ask = std::max(y1_ask, minY);
         y1_ask = std::min(y1_ask, maxY);
         if (minPixelRealPrice > y1_ask)
            minPixelRealPrice = y1_ask;

         y1_bid = std::max(y1_bid, minY);
         y1_bid = std::min(y1_bid, maxY);
         if (maxPixelRealPrice < y1_bid)
            maxPixelRealPrice = y1_bid;

         val_ask = askPrices[i];
         val_bid = bidPrices[i];

         val_ask -= y_scale_shift;
         val_bid -= y_scale_shift;

         y2_ask = (int)(m_y_0 - val_ask * m_scale_y + 0.5);
         y2_bid = (int)(m_y_0 - val_bid * m_scale_y + 0.5);

         y2_ask = std::max(y2_ask, minY);
         y2_ask = std::min(y2_ask, maxY);
         if (minPixelRealPrice > y2_ask)
            minPixelRealPrice = y2_ask;

         y2_bid = std::max(y2_bid, minY);
         y2_bid = std::min(y2_bid, maxY);
         if (maxPixelRealPrice < y2_bid)
            maxPixelRealPrice = y2_bid;

         rectDt1 = x;
         tmParamAnchorPt = x;
      }
      else
      {
         y1_ask = y2_ask;
         y1_bid = y2_bid;
         rectDt1 = x + dx;
         rectDt2 = rectDt1 + dx - 1;
         val_ask = askPrices[i];
         val_bid = bidPrices[i];
         tmParamAnchorPt = x + 1;

         val_ask -= y_scale_shift;
         val_bid -= y_scale_shift;

         y2_ask = (int)(m_y_0 - val_ask * m_scale_y + 0.5);
         y2_bid = (int)(m_y_0 - val_bid * m_scale_y + 0.5);

         y2_ask = std::max(y2_ask, minY);
         y2_ask = std::min(y2_ask, maxY);
         if (minPixelRealPrice > y2_ask)
            minPixelRealPrice = y2_ask;

         y2_bid = std::max(y2_bid, minY);
         y2_bid = std::min(y2_bid, maxY);
         if (maxPixelRealPrice < y2_bid)
            maxPixelRealPrice = y2_bid;
      }

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
                     DrawBitTimeSepStamp_09(transactionsDescriptions[transactionPStartInd], (int)transactionsTab[transactionPStartInd][3],
                                            x - (((int)transactionsTab[transactionPStartInd][3] * 5) / 2), rct.top, m_data_area.left, m_data_area.right);

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
      if (travelledDistanceVis)
      {
         if (counter == 1)
         {
            DrawTravelledDistanceScales();
         }
         int local_Y1 = m_data_area.top + 40 - travelledDistance[i - 1] * 2;
         int local_Y2 = m_data_area.top + 40 - travelledDistance[i] * 2;

         if (local_Y1 < minY)
            local_Y1 = minY;
         else if (local_Y1 > maxY)
            local_Y1 = maxY;
         if (local_Y2 < minY)
            local_Y2 = minY;
         else if (local_Y2 > maxY)
            local_Y2 = maxY;

         if (tickSampleWidth == 1)
         {
            if (local_Y1 == local_Y2)
               PixelSet(x, local_Y1, travelledDistanceColor);
            else
               LineVertical(x, local_Y1, local_Y2, travelledDistanceColor);
         }
         else
         {
            if (local_Y1 == local_Y2)
               LineHorizontal(x, x + dx, local_Y1, travelledDistanceColor);
            else
               SafeSlopingLine(x, local_Y1, x + dx, local_Y2, travelledDistanceColor);
         }
      }
      if (travelledRoadVis)
      {
         int local_Y1 = m_data_area.bottom - (travelledRoad[i - 1] * 2);
         int local_Y2 = m_data_area.bottom - (travelledRoad[i] * 2);

         if (local_Y1 < minY)
            local_Y1 = minY;
         else if (local_Y1 > maxY)
            local_Y1 = maxY;
         if (local_Y2 < minY)
            local_Y2 = minY;
         else if (local_Y2 > maxY)
            local_Y2 = maxY;

         if (tickSampleWidth == 1)
         {
            if (local_Y1 == local_Y2)
               PixelSet(x, local_Y1, travelledRoadColor);
            else
               LineVertical(x, local_Y1, local_Y2, travelledRoadColor);
         }
         else
         {
            if (local_Y1 == local_Y2)
               LineHorizontal(x, x + dx, local_Y1, travelledRoadColor);
            else
               SafeSlopingLine(x, local_Y1, x + dx, local_Y2, travelledRoadColor);
         }
         if (counter == 1)
         {
            DrawTravelledRoadScales();
         }
      }
      if (ticksArrivedVis)
      {
         int local_Y1 = m_data_area.bottom - (ticksArrived[i - 1] * 2);
         int local_Y2 = m_data_area.bottom - (ticksArrived[i] * 2);

         if (local_Y1 < minY)
            local_Y1 = minY;
         else if (local_Y1 > maxY)
            local_Y1 = maxY;
         if (local_Y2 < minY)
            local_Y2 = minY;
         else if (local_Y2 > maxY)
            local_Y2 = maxY;

         if (tickSampleWidth == 1)
         {
            if (local_Y1 == local_Y2)
               PixelSet(x, local_Y1, ticksArrivedColor);
            else
               LineVertical(x, local_Y1, local_Y2, ticksArrivedColor);
         }
         else
         {
            if (local_Y1 == local_Y2)
               LineHorizontal(x, x + dx, local_Y1, ticksArrivedColor);
            else
               SafeSlopingLine(x, local_Y1, x + dx, local_Y2, ticksArrivedColor);
         }
         if (!travelledRoadVis)
         {
            if (counter == 1)
            {
               DrawTicksArrivedScales();
            }
         }
      }

      if (interval > 0)
      {
         // Modulo expensive
         // C = A % B is equivalent to C = A � B * (A / B).
         // Instead of A/B:
         // precalc =ceil(2^32/B)
         //(A / B) == ((A * precalc) >> 32)
         uint prevTickIdx = i - 1; // times[i-1];
         if (tickSampleWidth == 1 && counter > 1)
            --prevTickIdx; // times[i-2];

         time_t _tm = times[i];
         ulong secs = _tm - dayHourSeconds1;
         if (secs >= 3600)
         {
            secs = _tm;
            uint days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
            secs -= (ulong)days * 24 * 60 * 60;
            hour1 = (secs * 1193047) >> 32;
            secs -= hour1 * 3600;
            dayHourSeconds1 = _tm - secs;
         }
         ulong mn1 = (secs * 71582789) >> 32;

         _tm = times[prevTickIdx];
         secs = _tm - dayHourSeconds2;
         if (secs >= 3600)
         {
            secs = _tm;
            uint days = (((_tm * 1158050442) >> 32) + _tm * 49710) >> 32;
            secs -= (ulong)days * 24 * 60 * 60;
            ulong _hr = (secs * 1193047) >> 32;
            secs -= _hr * 3600;
            dayHourSeconds2 = _tm - secs;
         }
         ulong mn2 = (secs * 71582789) >> 32;

         if (mn1 != mn2)
         {
            ulong a1 = (mn1 * interval_factor) >> 32;
            if ((mn1 - a1 * interval) == 0)
            {
               SafeSortedLineVertical(x + dx, 10, m_data_area.bottom, timeIntervalObjColor);
               erase_bkg_ver[x + dx] = 1;

               timeSepSignature[0] = TIMETABLE_A[hour1][0];
               timeSepSignature[1] = TIMETABLE_A[hour1][1];
               timeSepSignature[3] = TIMETABLE_A[mn1][0];
               timeSepSignature[4] = TIMETABLE_A[mn1][1];

               DrawBitTimeSepStamp_09(timeSepSignature, 5, x - 11, 2, m_data_area.left + 1, m_data_area.right - 1);
            }
         }
      }

      if (drawVerticalGrid)
      {
         if (((tickSampleWidth == 1) && (counter % 40 == 0)) || ((tickSampleWidth == 2) && (counter % 20 == 0)) || ((tickSampleWidth == 4) && (counter % 10 == 0)))
            LineVerticalDott(rectDt1, 10, m_data_area.bottom, verticalGridColor);
      }

      if (mTimeParameterVis)
      {
         bool cond = timeParameters[i] > 0;
         if (tickSampleWidth == 1)
            cond = cond || (timeParameters[i - 1] > 0);

         if (cond)
         {
            int colorIdx = 0; // grey
            if (mColorTimeParameter)
               colorIdx = 1; // color

            ulong timeParamDiff = 0;
            if (tickSampleWidth == 1)
               timeParamDiff = ((timeParameters[i - 1] + timeParameters[i]) >> 1) - tmParamMin + 1;
            else
               timeParamDiff = timeParameters[i] - tmParamMin + 1;

            ulong _tmp_value = ((timeParamDiff * tmParamFactor) >> 21);

            if (_tmp_value > 255)
               _tmp_value = 255;

            ulong tmParamIdx = 255 - _tmp_value;

            tmParamClr = timeParamColors[tmParamIdx][colorIdx];
            ulong fillValue = *tmParamPaddingClr;
            fillValue = fillValue << 32 | tmParamClr;

            uint index = (m_data_area.bottom - (LONG)tmParamFigureHight) * m_width + tmParamAnchorPt;
            for (uint i = tmParamFigureHight + 1; i--; index += m_width)
            {
               *(ulong *)(&m_pixels[index]) = fillValue;
            }
         }
      }
      if (tickSampleWidth > 1)
      {
         if (mCumulativeDataAskVis)
         {
            if (chartVisibility[0] == 1)
            {
               if (mCumulativeCountDataAsk[i] != 0)
               {
                  int maxAskInd;
                  int minAskInd;
                  MinMax(mCumulativeCountDataAsk, extremumStartIndex, extremumCount, minAskInd, maxAskInd);

                  int extremumInd = abs(maxAskInd) > abs(minAskInd) ? abs(maxAskInd) : abs(minAskInd);

                  int anchorPoint1 = 60;
                  int anchorPoint2 = anchorPoint1 - (mCumulativeCountDataAsk[i] * 20) / extremumInd;

                  if (tickSampleWidth == 4)
                     FillRectangle(rectDt1, anchorPoint1, rectDt2, anchorPoint2, cumulativeAskDataColor);
                  else
                  {
                     if (anchorPoint1 > anchorPoint2)
                     {
                        anchorPoint1 += anchorPoint2;
                        anchorPoint2 = anchorPoint1 - anchorPoint2;
                        anchorPoint1 -= anchorPoint2;
                     }
                     SafeSortedLineVertical(rectDt1, anchorPoint1, anchorPoint2, cumulativeAskDataColor);
                  }
               }
            }
         }
         if (mCumulativeDataBidVis)
         {
            if (chartVisibility[1] == 1)
            {
               if (mCumulativeCountDataBid[i] != 0)
               {
                  int maxBidInd;
                  int minBidInd;
                  MinMax(mCumulativeCountDataBid, extremumStartIndex, extremumCount, minBidInd, maxBidInd);

                  int extremumInd = abs(maxBidInd) > abs(minBidInd) ? abs(maxBidInd) : abs(minBidInd);
                  int anchorPoint1 = 60;
                  if (chartVisibility[0] == 1 && mCumulativeDataAskVis)
                     anchorPoint1 = 100;

                  int anchorPoint2 = anchorPoint1 - (mCumulativeCountDataBid[i] * 20) / extremumInd;
                  if (tickSampleWidth == 4)
                     FillRectangle(rectDt1, anchorPoint1, rectDt2, anchorPoint2, cumulativeBidDataColor);
                  else
                  {
                     if (anchorPoint1 > anchorPoint2)
                     {
                        anchorPoint1 += anchorPoint2;
                        anchorPoint2 = anchorPoint1 - anchorPoint2;
                        anchorPoint1 -= anchorPoint2;
                     }
                     SafeSortedLineVertical(rectDt1, anchorPoint1, anchorPoint2, cumulativeBidDataColor);
                  }
               }
            }
         }
      }

      if (mainPlotVis)
      {
         if (chartVisibility[0] == 1)
         {
            if (tickSampleWidth > 1)
            {
               if (y1_ask != y2_ask)
               {
                  SafeSlopingLine(x, y1_ask, x + dx, y2_ask, askLineColor);
               }
               else
               {
                  if ((unsigned)(y1_ask - minY - 1) < valid_range)
                  {
                     uint clr = askLineColor;
                     uint *data = &m_pixels[y1_ask * m_width + x];
                     *data = clr;
                     *(data + 1) = clr;
                     if (dx > 1)
                     {
                        *(data + 2) = clr;
                     }
                  }
               }
            }
            else
            {
               // Here we can sort points, as in the next iteration we read both new values
               if (y1_ask > y2_ask)
               {
                  y1_ask += y2_ask;
                  y2_ask = y1_ask - y2_ask;
                  y1_ask -= y2_ask;
               }
               if (lastAsk[0] >= 0 && lastAsk[1] >= 0)
               {
                  currentAsk[0] = y1_ask;
                  currentAsk[1] = y2_ask;

                  if (currentAsk[1] < lastAsk[0])
                     SafeSortedLineVertical(x, currentAsk[0], lastAsk[0], askLineColor);
                  else if (currentAsk[0] > lastAsk[1])
                     SafeSortedLineVertical(x, lastAsk[1], currentAsk[1], askLineColor);
                  else
                  {
                     if (y1_ask != y2_ask)
                        SafeSortedLineVertical(x, y1_ask, y2_ask, askLineColor);
                     else if ((unsigned)(y1_ask - minY - 1) < valid_range)
                        m_pixels[y1_ask * m_width + x] = askLineColor;
                  }
               }
               else
               {
                  if (y1_ask != y2_ask)
                     SafeSortedLineVertical(x, y1_ask, y2_ask, askLineColor);
                  else if ((unsigned)(y1_ask - minY - 1) < valid_range)
                     m_pixels[y1_ask * m_width + x] = askLineColor;
               }
               lastAsk[0] = y1_ask;
               lastAsk[1] = y2_ask;
            }
         }
         if (chartVisibility[1] == 1)
         {
            if (tickSampleWidth > 1)
            {
               if (y1_bid != y2_bid)
               {
                  SafeSlopingLine(x, y1_bid, x + dx, y2_bid, bidLineColor);
               }
               else
               {
                  if ((unsigned)(y1_bid - minY - 1) < valid_range)
                  {
                     uint clr = bidLineColor;
                     uint *data = &m_pixels[y1_bid * m_width + x];
                     *data = clr;
                     *(data + 1) = clr;
                     if (dx > 1)
                     {
                        *(data + 2) = clr;
                     }
                  }
               }
            }
            else
            {
               if (y1_bid > y2_bid)
               {
                  y1_bid += y2_bid;
                  y2_ask = y1_bid - y2_bid;
                  y1_bid -= y2_bid;
               }
               if (lastBid[0] > -1 && lastBid[1] > -1)
               {
                  currentBid[0] = y1_bid;
                  currentBid[1] = y2_bid;

                  if (currentBid[1] < lastBid[0])
                     SafeSortedLineVertical(x, currentBid[0], lastBid[0], bidLineColor);
                  else if (currentBid[0] > lastBid[1])
                     SafeSortedLineVertical(x, lastBid[1], currentBid[1], bidLineColor);
                  else
                  {
                     if (y1_bid != y2_bid)
                        SafeSortedLineVertical(x, y1_bid, y2_bid, bidLineColor);
                     else if ((unsigned)(y1_bid - minY - 1) < valid_range)
                        m_pixels[y1_bid * m_width + x] = bidLineColor;
                  }
               }
               else
               {
                  if (y1_bid != y2_bid)
                     SafeSortedLineVertical(x, y1_bid, y2_bid, bidLineColor);
                  else if ((unsigned)(y1_bid - minY - 1) < valid_range)
                     m_pixels[y1_bid * m_width + x] = bidLineColor;
               }
               lastBid[0] = y1_bid;
               lastBid[1] = y2_bid;
            }
         }
      }
      counter++;
   }

   time_t t1 = times[extremumStartIndex + 1];
   time_t t2 = times[seriesPointer];
   // C = A % B is equivalent to C = A � B * (A / B).
   // Based on Jacob Vecht's post at https://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/
   // Nearly 2 times less assembly than code commented below
   uint days = (((t1 * 1158050442) >> 32) + t1 * 49710) >> 32;
   t1 -= (ulong)days * 24 * 60 * 60;
   uint hr = (t1 * 1193047) >> 32;
   t1 -= (ulong)hr * 3600;
   uint mn = t1 * 71582789 >> 32;
   uint sec = (uint)(t1 - (ulong)mn * 60);

   TIMESTAMP_SHORT[0] = TIMETABLE_A[hr][0];
   TIMESTAMP_SHORT[1] = TIMETABLE_A[hr][1];
   TIMESTAMP_SHORT[3] = TIMETABLE_A[mn][0];
   TIMESTAMP_SHORT[4] = TIMETABLE_A[mn][1];
   TIMESTAMP_SHORT[6] = TIMETABLE_A[sec][0];
   TIMESTAMP_SHORT[7] = TIMETABLE_A[sec][1];

   DrawBitText_12(TIMESTAMP_SHORT, 8, m_data_area.left + 6, 25, false);

   // hr = int((t2%86400)/3600);
   // mn = int((t2%3600)/60);
   // sec = int((t2%60));
   days = (((t2 * 1158050442) >> 32) + t2 * 49710) >> 32;
   t2 -= (ulong)days * 24 * 60 * 60;
   hr = (t2 * 1193047) >> 32;
   t2 -= (ulong)hr * 3600;
   mn = t2 * 71582789 >> 32;
   sec = (int)(t2 - (ulong)mn * 60);

   TIMESTAMP_SHORT[0] = TIMETABLE_A[hr][0];
   TIMESTAMP_SHORT[1] = TIMETABLE_A[hr][1];
   TIMESTAMP_SHORT[3] = TIMETABLE_A[mn][0];
   TIMESTAMP_SHORT[4] = TIMETABLE_A[mn][1];
   TIMESTAMP_SHORT[6] = TIMETABLE_A[sec][0];
   TIMESTAMP_SHORT[7] = TIMETABLE_A[sec][1];

   DrawBitText_12(TIMESTAMP_SHORT, 8, m_data_area.left + 6, 36, false);

   memset(&erase_bkg_hor[minPixelRealPrice], 1, (maxPixelRealPrice - minPixelRealPrice + 1) * sizeof(int));
   memset(&erase_bkg_hor[m_data_area.bottom - tmParamFigureHight], 1, (tmParamFigureHight + 1) * sizeof(int));

   vScaleParamsChanged = false;
}

void CTickChart::DrawSignedLevels(void)
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
void CTickChart::DrawMProfile(int dataStartIndex)
{
   if (mProfileSize > 1)
   {
      bool showAsk = mProfileDataAskVis && (chartVisibility[0] == 1);
      bool showBid = mProfileDataBidVis && (chartVisibility[1] == 1);
      if (showAsk | showBid)
      {
         int rangeHeight = m_y_min + 1;
         ulong dyRaw = (((long)m_y_min - (long)m_y_max) * mProfileSizeFactor) >> 31;
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
               delete[] rescaledMProfileAsk;
               delete[] rescaledMProfileBid;

               rescaledMProfileAsk = new double[rangeHeight];
               rescaledMProfileBid = new double[rangeHeight];
               rescaledMProfileTabSize = rangeHeight;
            }
            memset(rescaledMProfileAsk, 0, rangeHeight * sizeof(double));
            memset(rescaledMProfileBid, 0, rangeHeight * sizeof(double));

            double mostSignificantPricePart = int((bidPrices[dataStartIndex] * _DigitsMultiplier) / 100000) * POWER_OF_10[5 - _Digits];
            double vvv = NormalizeDouble(mProfileStartIndex * _Point + mostSignificantPricePart, _Digits);

            vvv -= vvvOffst;

            int yInd = (int)(m_y_0 - vvv * m_scale_y + 0.5);

            if ((uint)yInd < (uint)rangeHeight)
            {
               if (showAsk)
                  rescaledMProfileAsk[yInd] += mProfileDataAsk[mProfileStartIndex];

               if (showBid)
                  rescaledMProfileBid[yInd] += mProfileDataBid[mProfileStartIndex];
            }

            for (int i = mProfileStartIndex + 1; i < mProfileSize + mProfileStartIndex; i++)
            {
               vvv = i * _Point + mostSignificantPricePart;
               vvv -= vvvOffst;
               yInd = (int)(m_y_0 - vvv * m_scale_y + 0.5);

               if ((uint)yInd < (uint)rangeHeight)
               {
                  if (showAsk)
                     rescaledMProfileAsk[yInd] += mProfileDataAsk[i];

                  if (showBid)
                     rescaledMProfileBid[yInd] += mProfileDataBid[i];
               }
            }

            double maxBid = MaxInArray(rescaledMProfileBid, 0, rangeHeight);
            double maxAsk = MaxInArray(rescaledMProfileAsk, 0, rangeHeight);

            if (maxAsk > maxBid)
            {
               if (maxAsk != 0)
               {
                  multiplierValid = true;
                  multiplier = 1.0 / maxAsk;
               }
            }
            else
            {
               if (maxBid != 0)
               {
                  multiplierValid = true;
                  multiplier = 1.0 / maxBid;
               }
            }
         }
         else
         {
            double maxBid = MaxInArray(mProfileDataBid, mProfileStartIndex, mProfileSize);
            double maxAsk = MaxInArray(mProfileDataAsk, mProfileStartIndex, mProfileSize);
            multiplierValid = false;

            if (maxAsk > maxBid)
            {
               if (maxAsk != 0)
               {
                  multiplierValid = true;
                  multiplier = 1.0 / maxAsk;
               }
            }
            else
            {
               if (maxBid != 0)
               {
                  multiplierValid = true;
                  multiplier = 1.0 / maxBid;
               }
            }
         }

         if (multiplierValid)
         {
            if (dy == 0)
            {
               int new_x = 0;
               for (int i = m_y_max; i < rangeHeight; i++)
               {
                  if (showAsk && rescaledMProfileAsk[i] > 0)
                  {
                     new_x = (m_data_area.left + (int)(((double)rescaledMProfileAsk[i] * multiplier) * 200.0 + 0.5));
                     SafeSortedLineHorizontal(m_data_area.left + 1, new_x, i, mProfileAskColor);
                  }
                  if (showBid && rescaledMProfileBid[i] > 0)
                  {
                     new_x = (m_data_area.left + (int)(((double)rescaledMProfileBid[i] * multiplier) * 200.0 + 0.5));
                     SafeSortedLineHorizontal(m_data_area.left + 1, new_x, i, mProfileBidColor);
                  }
               }
            }
            else
            {
               double mostSignificantPricePart = int((bidPrices[dataStartIndex] * _DigitsMultiplier) / 100000) * POWER_OF_10[5 - _Digits];
               double vvv = mProfileStartIndex * _Point + mostSignificantPricePart;

               vvv -= vvvOffst;
               int yyy2 = (int)(m_y_0 - vvv * m_scale_y + 0.5);

               int new_x = 0;

               if (showAsk && mProfileDataAsk[mProfileStartIndex] > 0)
               {
                  new_x = (m_data_area.left + (int)(((double)mProfileDataAsk[mProfileStartIndex] * multiplier) * 200.0 + 0.5));
                  SafeSortedFillRectangle(m_data_area.left + 1, yyy2 + 1 - dy, new_x, yyy2, mProfileAskColor);
               }
               if (showBid && mProfileDataBid[mProfileStartIndex] > 0)
               {
                  new_x = (m_data_area.left + (int)(((double)mProfileDataBid[mProfileStartIndex] * multiplier) * 200.0 + 0.5));
                  SafeSortedFillRectangle(m_data_area.left + 1, yyy2 + 1 - dy, new_x, yyy2, mProfileBidColor);
               }

               for (int i = mProfileStartIndex + 1; i < mProfileSize + mProfileStartIndex; i++)
               {
                  int yyy1 = yyy2;
                  vvv = 0;
                  vvv = i * _Point + mostSignificantPricePart;
                  vvv -= m_v_scale_min;
                  yyy2 = (int)(m_y_0 - vvv * m_scale_y + 0.5);

                  int finalY1 = yyy1 - dy;
                  int finalY2 = yyy2 + 1 - dy;
                  if (finalY1 > finalY2)
                  {
                     finalY1 += finalY2;
                     finalY2 = finalY1 - finalY2;
                     finalY1 -= finalY2;
                  }

                  if (showAsk && mProfileDataAsk[i] > 0)
                  {
                     new_x = (m_data_area.left + (int)(((double)mProfileDataAsk[i] * multiplier) * 200.0 + 0.5));
                     SafeSortedFillRectangle(m_data_area.left + 1, finalY1, new_x, finalY2, mProfileAskColor);
                  }
                  if (showBid && mProfileDataBid[i] > 0)
                  {
                     new_x = (m_data_area.left + (int)(((double)mProfileDataBid[i] * multiplier) * 200.0 + 0.5));
                     SafeSortedFillRectangle(m_data_area.left + 1, finalY1, new_x, finalY2, mProfileBidColor);
                  }
               }
            }
         }
      }
   }
}

void CTickChart::DrawTravelledDistanceScales()
{
   if (!travelledDistanceScaleDrawn || vScaleParamsChanged)
   {
      char txt[3] = "";

      txt[0] = '-';
      txt[1] = '2';
      DrawBitText_12(txt, 2, m_data_area.left - 2 - 7 * 2, m_data_area.top + 45, false);

      txt[0] = '2';
      DrawBitText_12(txt, 1, m_data_area.left - 2 - 7 * 1, m_data_area.top + 25, false);

      txt[0] = '-';
      txt[1] = '4';
      DrawBitText_12(txt, 2, m_data_area.left - 2 - 7 * 2, m_data_area.top + 55, false);

      txt[0] = '4';
      DrawBitText_12(txt, 1, m_data_area.left - 2 - 7 * 1, m_data_area.top + 15, false);

      txt[0] = '-';
      txt[1] = '2';
      DrawBitText_12(txt, 2, m_data_area.right + 6 - 7 * 2, m_data_area.top + 45, false);

      txt[0] = '2';
      DrawBitText_12(txt, 1, m_data_area.right + 6 - 7 * 1, m_data_area.top + 25, false);

      txt[0] = '-';
      txt[1] = '4';
      DrawBitText_12(txt, 2, m_data_area.right + 6 - 7 * 2, m_data_area.top + 55, false);

      txt[0] = '4';
      DrawBitText_12(txt, 1, m_data_area.right + 6 - 7 * 1, m_data_area.top + 15, false);

      travelledDistanceScaleDrawn = true;
   }

   LineHorizontal(m_data_area.left + 1, m_data_area.right - 10, (m_data_area.top + 40), travelledDistanceHLineColor);
   LineHorizontal(m_data_area.left + 1, m_data_area.right - 10, (m_data_area.top + 50), travelledDistanceHLineColor);
   LineHorizontal(m_data_area.left + 1, m_data_area.right - 10, (m_data_area.top + 60), travelledDistanceHLineColor);
   LineHorizontal(m_data_area.left + 1, m_data_area.right - 10, (m_data_area.top + 30), travelledDistanceHLineColor);
   LineHorizontal(m_data_area.left + 1, m_data_area.right - 10, (m_data_area.top + 20), travelledDistanceHLineColor);
}
void CTickChart::DrawTravelledRoadScales()
{
   if (travelledRoadScaleDrawn && !vScaleParamsChanged)
      return;

   DrawingHelper();
   travelledRoadScaleDrawn = true;
   ticksArrivedScaleDrawn = true;
}
void CTickChart::DrawTicksArrivedScales()
{
   if (ticksArrivedScaleDrawn && !vScaleParamsChanged)
      return;

   DrawingHelper();
   ticksArrivedScaleDrawn = true;
   travelledRoadScaleDrawn = true;
}
void CTickChart::DrawingHelper()
{
   char txt[3] = "";
   int letterWidth = 7;
   int off1 = m_data_area.left - 2 - letterWidth;

   txt[0] = '2';
   DrawBitText_12(txt, 1, off1, m_data_area.bottom - 15, false);
   txt[0] = '6';
   DrawBitText_12(txt, 1, off1, m_data_area.bottom - 35, false);
   txt[0] = '1';
   txt[1] = '0';
   off1 = m_data_area.left - 2 - letterWidth * 2;
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 55, false);
   txt[1] = '4';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 75, false);
   txt[1] = '8';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 95, false);
   txt[0] = '2';
   txt[1] = '2';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 115, false);
   txt[1] = '6';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 135, false);
   txt[0] = '3';
   txt[1] = '0';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 155, false);

   off1 = m_data_area.right + 6 - letterWidth * 1;
   txt[0] = '2';
   DrawBitText_12(txt, 1, off1, m_data_area.bottom - 15, false);
   txt[0] = '6';
   DrawBitText_12(txt, 1, off1, m_data_area.bottom - 35, false);
   txt[0] = '1';
   txt[1] = '0';
   off1 = m_data_area.right + 6 - letterWidth * 2;
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 55, false);
   txt[1] = '4';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 75, false);
   txt[1] = '8';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 95, false);
   txt[0] = '2';
   txt[1] = '2';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 115, false);
   txt[0] = '2';
   txt[1] = '6';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 135, false);
   txt[0] = '3';
   txt[1] = '0';
   DrawBitText_12(txt, 2, off1, m_data_area.bottom - 155, false);
}
// x1, x2 - sorted (always x1<x2)
void CTickChart::SafeSlopingLine(int x1, int y1, int x2, int y2, const uint clr)
{
   int dx = x2 - x1; // always >=0
   int dy = y2 - y1;

   int sx = 1;
   int sy = 1;

   if (dy < 0)
   {
      dy = -dy;
      sy = -1;
   }
   int er = dx - dy;
   bool draw = false;

   while (x1 != x2 || y1 != y2)
   {
      if ((unsigned)x1 < (unsigned)m_width &&
          (unsigned)(y1 - 2) < (unsigned)(m_height - 1))
      {
         m_pixels[y1 * m_width + x1] = clr;
         draw = true;
      }
      else
      {
         if (draw)
            return;
      }
      int er2 = er << 1;
      if (er2 > -dy)
      {
         er -= dy;
         x1 += sx;
      }
      if (er2 < dx)
      {
         er += dx;
         y1 += sy;
      }
   }
   m_pixels[y2 * m_width + x2] = clr;
}