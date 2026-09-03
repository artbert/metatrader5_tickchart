// Used by mt5_tick_chart.rc

#define IDM_TICK_CHART_ZOOM_4 202
#define IDM_TICK_CHART_ZOOM_2 201
#define IDM_TICK_CHART_ZOOM_1 200
#define IDM_BAR_CHART_CANDLE_WIDTH_4 205
#define IDM_BAR_CHART_CANDLE_WIDTH_2 204
#define IDM_BAR_CHART_CANDLE_WIDTH_1 203
#define IDM_TICK_TIME_SEP_0 206
#define IDM_TICK_TIME_SEP_1 207
#define IDM_TICK_TIME_SEP_5 208
#define IDM_TICK_TIME_SEP_15 209
#define IDM_TICK_TIME_SEP_30 210
#define IDM_TICK_TIME_SEP_60 211
#define IDM_BAR_TIME_SEP_0 212
#define IDM_BAR_TIME_SEP_1 213
#define IDM_BAR_TIME_SEP_5 214
#define IDM_BAR_TIME_SEP_15 215
#define IDM_BAR_TIME_SEP_30 216
#define IDM_BAR_TIME_SEP_60 217
#define IDM_FREEZE_RANGES 218
#define IDM_RESET_CHARTS 219
#define IDM_VIS_ASK 220
#define IDM_VIS_BID 221
#define IDM_VIS_LEVELS 222
#define IDM_VIS_AUTO_MOV_RANGE 223
#define IDM_VIS_MPROFILE_ASK 224
#define IDM_VIS_MPROFILE_BID 225
#define IDM_VIS_TICK_CHART 226
#define IDM_VIS_BAR_CHART 227
#define IDM_VIS_TIME_PARAMETER 228
#define IDM_VIS_CUMULATIVE_ASK 229
#define IDM_VIS_CUMULATIVE_BID 230
#define IDM_VIS_DISTANCE 231
#define IDM_VIS_ROAD 232
#define IDM_VIS_TICKS 233
#define IDM_BAR_CHART_EVENTS 234
#define IDM_TICK_CHART_EVENTS 235
#define IDM_ORDERS 236
#define IDM_COLOR_TIME_PARAM 237
#define IDM_ZOOM_TIME_PARAM 238
#define IDM_SAVE_TICKS 239
#define IDM_PRINT_SCREEN 240
#define IDM_TIMER_2000 251
#define IDM_TIMER_1000 250
#define IDM_TIMER_900 249
#define IDM_TIMER_800 248
#define IDM_TIMER_700 247
#define IDM_TIMER_600 246
#define IDM_TIMER_500 245
#define IDM_TIMER_400 244
#define IDM_TIMER_300 243
#define IDM_TIMER_200 242
#define IDM_TIMER_100 241
#define IDM_PLAY_REAL_TEMPO 252

#define IDC_DATELABEL 300
#define IDC_INFOLABEL 302
#define IDC_LD_FROM_SERVER_BTN 303
#define IDC_CHART_AUTO_SCRL_CHBX 304
#define IDC_STEP_SIZE_EDT 305
#define IDC_STEP_FORWRD_BTN 306
#define IDC_STEP_BACKWRD_BTN 307
#define IDC_MEASURE_CHBX 308
#define IDC_REPLAY_BTN 309
#define IDC_MAIN_EDT 310
#define IDC_SET_RANGE_BTN 311
#define IDC_SET_BAR_T_SIZE_BTN 312
#define IDC_MV_RANGE_UP_BTN 313
#define IDC_MV_RANGE_DOWN_BTN 314
#define IDC_LVLS_EDIT_CHBX 315
#define IDC_LEVEL_ADD_BTN 316
#define IDC_LEVELS_DEL_BTN 317
#define IDC_LD_FROM_SERVER_PARTIAL_BTN 318

#define IDS_APP_TITLE 103

#define IDR_MAINFRAME 128
#define IDD_MT5TICKCHART_DIALOG 102
#define IDD_ABOUTBOX 103
#define IDM_ABOUT 104
#define IDM_EXIT 105
#define IDI_MT5TICKCHART 107
#define IDI_SMALL 108
#define IDC_MT5TICKCHART 109

#define IDD_TOOLBAR 111

#define ID_TOOLBAR_SHOW 112

#define IDC_MYICON 2
#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif
// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NO_MFC 130
#define _APS_NEXT_RESOURCE_VALUE 129
#define _APS_NEXT_COMMAND_VALUE 32771
#define _APS_NEXT_CONTROL_VALUE 1000
#define _APS_NEXT_SYMED_VALUE 110
#endif
#endif

#pragma once

typedef long long time_t;
typedef struct TICKCHARTMODULESETTINGS
{
   unsigned char tickChartZoom;
   unsigned char barChartCandleWidth;
   unsigned char tickChartTimeSep;
   unsigned char barChartTimeSep;
   bool freezeRanges;
   bool askLineVis;
   bool bidLineVis;
   bool signedLevelsVis;
   bool autoMovingRange;
   bool mProfileAskVis;
   bool mProfileBidVis;
   bool tickChartVis;
   bool barChartVis;
   bool timeParamVis;
   bool cumulativeAskVis;
   bool cumulativeBidVis;
   bool distanceVis;
   bool roadVis;
   bool ticksArrivedVis;
   bool eventsOnTickChartVis;
   bool eventsOnBarChartVis;
   bool ordersVis;
   bool colorTimeParam;
   bool zoomTimeParam;
   bool realTempo;
   bool chartAutoScroll;
   bool measurementTool;
   bool replayModeOn;
   int timerInterval;
   int tickOffsetValue;
   int barChartTickSize;

} TCHMODSET, *PTCHMODSET;

#pragma pack(push, 1)
typedef struct SYMBOLPARAMETERS
{
   double pointValue;
   unsigned int digitsCount;
   int pipsDivider;
   bool excludePremarketData;
   bool expandDateRange;
   bool darkMode;
   bool enableSpeedStats;
   double initAsk;
   double initBid;
   time_t initTime;
} SYMBOLSETS;

typedef struct MQL5TICKDATA
{
   time_t time;               // Time of the last prices update
   double bid;                // Current Bid price
   double ask;                // Current Ask price
   double last;               // Price of the last deal (Last)
   unsigned long long volume; // Volume for the current Last price
   long long time_msc;        // Time of a price last update in milliseconds
   unsigned int flags;        // Tick flags
   double volume_real;        // Volume for the current Last price with greater accuracy
} MqlTick;
#pragma pack(pop)

#define CLOSE_CHART 1000
#define SELECT_SIGNED_LEVELS 1001
#define UNSELECT_SIGNED_LEVELS 1002
#define DELETE_ALL_SIGNED_LEVELS 1003
#define ADD_SIGNED_LEVEL 1004
#define LOAD_DATA_FROM_SERVER 1005
#define SET_RANGE 1006
#define MOVE_RANGE_UP 1007
#define MOVE_RANGE_DOWN 1008
#define SHOW_ORDERS 1009
#define SHOW_SIGNED_LEVELS 1010
#define MODULE_INITIALIZED 1011
#define LOAD_DATA_FROM_SERVER_PARTIAL 1012
#define AUTO_SCROLL_CHANGED 1013
#define CURRENT_TIME_PRICE_CHANGED 1014

#define UPDATE_CHARTS (WM_APP + 1)
#define REWIND_CHARTS (WM_APP + 2)
#define AUTO_SCROLL_UPDATE (WM_APP + 3)

extern long long ProccessorFrequency;
long long GetMicrosecondCount();
