#pragma once
#include "chart_canvas.hpp"
#include <string>

using std::string;

class CBarChart : public ChartCanvas
{
private:
   double _Point{};
   double _DigitsMultiplier{};
   int _Digits{};

   bool DarkMode;
   int interval;
   int workSpaceChartHeightInPx;
   double pointsPerPx;

   double actualBidPrice, previousBidPrice, actualBidHigh, actualBidLow;
   double lastChartPriceMax, lastChartPriceMin;
   bool mProfileDataVis;
   bool mTimeParameterVis{};
   bool mColorTimeParameter{};
   bool mSignedLevelsVis{};
   bool mainPlotVis;

   bool initialized;
   bool drawVerticalGrid;

   char (*signedLevelsDescriptions)[100];
   double *signedLevels;
   int signedLevelsArraySize;
   bool isSignedLevelsDescriptions;
   uint barsDataMultiplier;

   double *openPrices, *closePrices, *highPrices, *lowPrices;
   ulong *timeParameters;
   time_t *times;
   int *mProfileData;
   int seriesPointer;
   int chartWidthInSamples;
   int totalMProfileSize;

   int seriesSize;
   int barChartTickSize;
   int barChartBarWidth;
   int mProfileStartIndex, mProfileSize;
   ulong mProfileSizeFactor;

   CalendarEvent *calendarEvents;
   int calendarEventsTabSize;
   bool isCalendarEvents, showCalendarEvents;

   bool isOrdersPoints{};
   long long (*transactionsTab)[4];
   char (*transactionsDescriptions)[64];
   int transactionsTabSize;

   int pipsDivider;
   int doubleSignificantPlaces{};

   uint signedLevelsObjColor{};
   uint signedLevelsTextColor{};
   uint timeParameterObjColor{};
   uint bullCandleColor{}, bearCandleColor{};
   uint timeIntervalTextColor{}, timeIntervalObjColor{};
   uint verticalGridColor{};
   uint orderPointBuyColor{}, orderPointSellColor{}, orderPointCloseColor{}, orderPointLineColor{}, orderPointTextColor{};
   uint calendarEventCircleColor{};
   uint mProfileColor{};

   uint mDataAreaStartPoint;

   char INFOSTRING[1000]{};
   char DESCRIPTION[100]{};
   uint timeParamColors[256][2]{};

   int extremumCount, extremumStartIndex;

   double *rescaledMProfile;
   uint rescaledMProfileTabSize;

   unsigned long long intervals_quot[5]{};
   int interval_idx;

public:
   CBarChart();
   ~CBarChart();

   virtual bool Create(HWND hWnd, const int width, const int height, double pointValue, int digits);
   void UpdateChart(bool vScaleParChanged = false);
   void RefreshWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
   void RefreshWindow();
   void DrawVerticalGrid(bool value)
   {
      drawVerticalGrid = value;
   }

   void SetInterval(const int value, const bool update = true)
   {
      interval = value;
      switch (interval)
      {
      case 1:
         interval_idx = 0;
         break;
      case 5:
         interval_idx = 1;
         break;
      case 15:
         interval_idx = 2;
         break;
      case 30:
         interval_idx = 3;
         break;
      case 60:
         interval_idx = 4;
      }
      if (update)
      {
         Redraw();
      }
   }
   void ShowOrderPoint(const bool value, const bool update = true)
   {
      isOrdersPoints = value;
      if (update)
      {
         Redraw();
      }
   }
   void SetPipsDivider(const int value)
   {
      pipsDivider = value;
      doubleSignificantPlaces = (int)log10((double)pipsDivider);
   }
   void SetColorMode(const bool value);
   void SetBarChartTickSize(const int value, const bool update = true)
   {
      barChartTickSize = value;
      vScaleParamsChanged = true;
      _itoa_s(barChartTickSize, DESCRIPTION, 100, 10);

      strcat_s(DESCRIPTION, sizeof(DESCRIPTION), "ABC");

      size_t len = strlen(DESCRIPTION);

      DESCRIPTION[len] = ' ';
      DESCRIPTION[len + 1] = 't';
      DESCRIPTION[len + 2] = '.';
      DESCRIPTION[len + 3] = '\0';

      if (update)
      {
         UpdateChart();
      }
   }
   void SetBarChartBarWidth(const int value, const bool update = true)
   {
      barChartBarWidth = value;

      extremumCount = chartWidthInSamples / barChartBarWidth;

      if (update)
      {
         UpdateChart();
      }
   }
   void ShowTimeParameter(bool value, const bool update = true)
   {
      mTimeParameterVis = value;
      if (update)
      {
         Redraw();
      }
   }
   void ColorTimeParameter(bool value, const bool update = true)
   {
      mColorTimeParameter = value;
      if (update)
      {
         Redraw();
      }
   }
   void ShowSignedLevels(bool value, const bool update = true)
   {
      mSignedLevelsVis = value;
      vScaleParamsChanged = true;
      if (update)
      {
         Redraw();
      }
   }
   void ShowMainPlot(bool value, const bool update = true)
   {
      mainPlotVis = value;
      if (update)
      {
         Redraw();
      }
   }
   void AppendSignedLevels(const double levels[], const int levelsSize, char (*descriptions)[100], const int descrSize, const bool update);
   void BiggerBarsData(uint value, const bool update = true)
   {
      barsDataMultiplier = value;
      if (update)
      {
         Redraw();
      }
   }
   void ShowMProfileData(bool value, const bool update = true)
   {
      mProfileDataVis = value;
      if (update)
      {
         UpdateChart();
      }
   }
   void UpdateMarketProfile(const double askPrice, const double bidPrice);
   void AppendPricesTimeAndParameters(const double open, const double close, const double high, const double low, const time_t time, const ulong timeParameter);
   void FillSeries(const double open, const double close, const double high, const double low, const time_t time, const ulong timeParameter, const bool update = true);

   void AppendCalendarEvents(CalendarEvent clEvents[], int tabSize, const bool update = true);
   void ShowCalendarEvents(bool value, const bool update = true)
   {
      showCalendarEvents = value;
      if (update)
      {
         UpdateChart();
      }
   }
   void AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size, const bool update = true);
   uint GetDataAreaStartPoint()
   {
      return (mDataAreaStartPoint);
   }
   double GetPointsPerPixel()
   {
      return (pointsPerPx);
   }

   void SetActualBidPrice(const double currentPrice, const double currentHigh, const double currentLow);
   void UpdateCurrentPriceLevel();

protected:
   void DrawChart() override;
   void DrawData(const uint index = 0) override;
   void DrawMProfile(int dataStartIndex);
   void DrawSignedLevels();
};