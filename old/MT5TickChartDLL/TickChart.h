#pragma once
#include "chartcanvas.h"
#include <string>
#include <math.h>
#include <algorithm>

#define IS_MPROFILEDATABID 1
#define IS_MPROFILEDATAASK 2
#define IS_CUMULATIVEDATAASK 4
#define IS_CUMULATIVEDATABID 8
#define IS_TRAVELLEDDISTANCE 16
#define IS_TRAVELLEDROAD 32
#define IS_TICKSARRIVED 64
#define IS_SHOW_TRANSACTIONS 128

//struct CalendarEvent
//{
//   time_t          eventDateTime;
//   wchar_t        eventSymbol[20];
//   byte            importance;
//   wchar_t        eventDescription[100];
//   short           eventChange;
//};

using std::string;

class CTickChart :
   public ChartCanvas {

 private:

   double _Point;
   int _Digits;
   double _DigitsMultiplier;
   bool              DarkMode;
   int               interval;
   bool              initialized;

   bool              mProfileDataBidVis;
   bool              mProfileDataAskVis;
   bool              mTimeParameterVis;
   bool              mColorTimeParameter;
   bool              mSignedLevelsVis;

   bool              mCumulativeDataBidVis;
   bool              mCumulativeDataAskVis;

   bool              boldenChart;
   bool              drawVerticalGrid;

   int               tickSampleWidth;
   bool              mainPlotVis;

   uint              barsDataMultiplier;

   int               *chartVisibility;
   uint              chartVisSize;

   double            *askPrices,*bidPrices;
   ulong             *timeParameters;
   char (*signedLevelsDescriptions)[100];
   char (*transactionsDescriptions)[64];
   double            *signedLevels;
   //std::vector<wchar_t *> signedLevelsDescriptions;
   //std::vector<double> signedLevels;
   int               signedLevelsArraySize;
   bool              isSignedLevelsDescriptions;
   int               *mCumulativeCountDataBid;
   int               *mCumulativeCountDataAsk;
   short             *travelledDistance;
   short             *travelledRoad;
   short             *ticksArrived;
   bool              travelledDistanceVis,travelledRoadVis,ticksArrivedVis;
   bool              travelledDistanceScaleDrawn,travelledRoadScaleDrawn,ticksArrivedScaleDrawn;
   int               *mProfileDataBid;
   int               *mProfileDataAsk;
   time_t            *times;
   int               seriesPointer;
   uint               chartWidthInSamples;

   int               totalMProfileSize;

   int               seriesSize;

   int               mProfileStartIndex,mProfileSize;
   ulong             mProfileSizeFactor;
   CalendarEvent *calendarEvents;
   int              calendarEventsTabSize;
   bool              isCalendarEvents,showCalendarEvents;

   bool              isOrdersPoints;
   long long (*transactionsTab)[4];        //open time-type-ticket-price in points
   int transactionsTabSize;

   char         INFOSTRING[1000];
   uint              timeParamColors[256][2];

   int               pipsDivider;
   int               doubleSignificantPlaces;

   uint              askLineColor;
   uint              bidLineColor;
   uint              cumulativeBidDataColor,cumulativeAskDataColor;
   uint              timeParameterObjColorNeutral,timeParameterObjColorBid,timeParameterObjColorAsk;
   uint              verticalGridColor;
   uint              duplicatedScaleColor;
   uint              timeIntervalTextColor,timeIntervalObjColor;
   uint              signedLevelsObjColor;
   uint              signedLevelsTextColor;
   uint              mProfileBidColor,mProfileAskColor;
   uint              calendarEventCircleColor;
   uint              orderPointBuyColor,orderPointSellColor,orderPointCloseColor,orderPointLineColor,orderPointTextColor;
   uint              travelledDistanceHLineColor;
   uint              travelledDistanceColor;
   uint              travelledRoadColor;
   uint              ticksArrivedColor;

   uint              mDataAreaStartPoint;

   int decimalSep;
   char TIMESTAMP_SHORT[9];

   int       *erase_bkg_hor;          // array of flags for indexes for horizontal lines to be erased
   int       *erase_bkg_ver;          // array of flags for indexes for vertical lines to be erased

   unsigned long long intervals_quot[5];
   int interval_idx;
   ulong erase_flags;
   uint _extremumCount;

   double *rescaledMProfileAsk;
   double *rescaledMProfileBid;
   uint rescaledMProfileTabSize;

 public:
   CTickChart(void);
   ~CTickChart(void);

   virtual bool Create(HWND hWnd,const int width,const int height, double pointValue, int digits);

   void SetInterval(const int value,const bool update=true)
   {
      interval=value;
      switch(interval)
      {
      case 1:
         interval_idx=0;
         break;
      case 5:
         interval_idx=1;
         break;
      case 15:
         interval_idx=2;
         break;
      case 30:
         interval_idx=3;
         break;
      case 60:
         interval_idx=4;
      }
      if(update)
      {
         Redraw();
      }
   }
   void SetPipsDivider(const int value)
   {
      pipsDivider=value;
      doubleSignificantPlaces=(int)log10((double)pipsDivider);
   }
   void ShowOrderPoint(const bool value,const bool update=true)
   {
      isOrdersPoints=value;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_SHOW_TRANSACTIONS;
      else
         erase_flags&=~IS_SHOW_TRANSACTIONS;
   }
   void SetColorMode(const bool value);
   void SetTickSampleWidth(const int value,const bool update=true)
   {
      tickSampleWidth=value;
      if(tickSampleWidth>1)
      {
         if(tickSampleWidth==2)
         {
            decimalSep=20;
            _extremumCount=chartWidthInSamples>>1;
         }
         else
         {
            decimalSep=10;
            _extremumCount=chartWidthInSamples>>2;
         }
      }
      else
      {
         decimalSep=40;
         _extremumCount=chartWidthInSamples;
      }

      if(update)
      {
         Redraw();
      }
   }
   void RefreshWindow(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
   void RefreshWindow();
   void UpdateChart(bool vScaleParChanged=false);
   void UpdateChart(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc,bool vScaleParChanged=false);
   void BoldenChart(bool value)
   {
      boldenChart=value;
   }
   void DrawVerticalGrid(bool value)
   {
      drawVerticalGrid=value;
   }
   void ChartVisibility(const uint pos,const int value,const bool update=true);
   void ShowMProfileDataBid(bool value,const bool update=true)
   {
      mProfileDataBidVis = value;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_MPROFILEDATABID;
      else
         erase_flags&=~IS_MPROFILEDATABID;
   }
   void ShowMProfileDataAsk(bool value,const bool update=true)
   {
      mProfileDataAskVis = value;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_MPROFILEDATAASK;
      else
         erase_flags&=~IS_MPROFILEDATAASK;
   }
   void ShowCumulativeDataAsk(bool value,const bool update=true)
   {
      mCumulativeDataAskVis = value;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_CUMULATIVEDATAASK;
      else
         erase_flags&=~IS_CUMULATIVEDATAASK;
   }
   void ShowCumulativeDataBid(bool value,const bool update=true)
   {
      mCumulativeDataBidVis = value;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_CUMULATIVEDATABID;
      else
         erase_flags&=~IS_CUMULATIVEDATABID;
   }
   void ShowTimeParameter(bool value,const bool update=true)
   {
      mTimeParameterVis=value;
      if(update)
      {
         Redraw();
      }
   }
   void ColorTimeParameter(bool value,const bool update=true)
   {
      mColorTimeParameter=value;
      if(update)
      {
         Redraw();
      }
   }
   void ShowSignedLevels(bool value,const bool update=true)
   {
      mSignedLevelsVis=value;
      vScaleParamsChanged=true;
      if(update)
      {
         Redraw();
      }
   }
   void AppendSignedLevels(const double levels[],const int levelsSize,char (*descriptions)[100],const int descrSize, const bool update);
   void ShowMainPlot(bool value,const bool update=true)
   {
      mainPlotVis=value;
      if(update)
      {
         Redraw();
      }
   }
   void BiggerBarsData(uint value,const bool update=true)
   {
      barsDataMultiplier=value;
      if(update)
      {
         Redraw();
      }
   }
   void FillSeries(const double askPrice,const double bidPrice,const time_t time,const uint timeParameter,const int cumulativeDataAsk,const int cumulativeDataBid);
   void AppendPricesTimeAndParameters(const double askPrice,const double bidPrice,const time_t time,const ulong timeParameter,const int askChange,const int bidChange,const short road,const short distance,const short ticks);
   void UpdateMarketProfile(const double askPrice,const double bidPrice);
   void MoveMarketProfileRange(const double lowRange,const double highRange);
   int GetSeriesSize(void)
   {
      return(chartWidthInSamples);
   }
   void AppendCalendarEvents(CalendarEvent clEvents[], int tabSize);
   void ShowCalendarEvents(bool value,const bool update=true)
   {
      showCalendarEvents=value;
      vScaleParamsChanged=true;
      if(update)
      {
         Redraw();
      }
   }
   void AppendTransactionsPoints(const long transactions[][4], char (*descriptions)[64], const int size);
   void ShowTravelledDistance(bool value,const bool update=true)
   {
      travelledDistanceVis=value;
      vScaleParamsChanged=true;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_TRAVELLEDDISTANCE;
      else
         erase_flags&=~IS_TRAVELLEDDISTANCE;
   }
   void ShowTravelledRoad(bool value,const bool update=true)
   {
      travelledRoadVis=value;
      vScaleParamsChanged=true;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_TRAVELLEDROAD;
      else
         erase_flags&=~IS_TRAVELLEDROAD;
   }
   void ShowTicksArrived(bool value,const bool update=true)
   {
      ticksArrivedVis=value;
      vScaleParamsChanged=true;
      if(update)
      {
         Redraw();
      }
      if(value)
         erase_flags|=IS_TICKSARRIVED;
      else
         erase_flags&=~IS_TICKSARRIVED;
   }
   uint GetDataAreaStartPoint(void)
   {
      return(mDataAreaStartPoint);
   }
 protected:
   virtual void DrawChart(void);
   virtual void DrawData(const uint index=0);
   virtual void DrawBackground(void);

 private:
   void SafeSlopingLine(int x1,int y1,int x2,int y2,const uint clr);
   void DrawSignedLevels(void);
   void DrawMProfile(int dataStartIndex);
   void DrawTravelledDistanceScales();
   void DrawTravelledRoadScales();
   void DrawTicksArrivedScales();
   void DrawingHelper();
};

