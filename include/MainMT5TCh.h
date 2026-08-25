#pragma once
#include "TickChartModule.h"
#include <Commdlg.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
TCHAR szTitle[MAX_LOADSTRING];       // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

unsigned int RandomRGB();
unsigned int RandomRGB_gdi();
// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int nCmdShow, HWND parentChart = NULL);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void ToolDlgFunctionsPanelProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
bool MenuItemsActivationProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TickChartWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BarChartWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditControlProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void CALLBACK TimerFunction(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

void InitTickChartModuleStruct(HWND hwnd);
bool SaveDataFile();
bool LoadDataFile();

LRESULT OldTickChartProc, OldBarChartProc, OldEditControlProc;
CTickChartModule chartModule;
int _RandomSeed = 11111;

HMENU hideShowHMenu, mainMenu;
HWND tickChartHWnd = NULL, barChartHWnd = NULL, mainWindowHandle = NULL;
HWND toolBarHWnd = NULL;
HWND MT5ParentChart = NULL;
HANDLE threadHandle = NULL;
WCHAR fullDataPath[MAX_PATH];
TCHMODSET appSets;
MMRESULT timerHandle = 0;

bool tickChMouseTracking = false, barChMouseTracking = false;

HINSTANCE hModuleInstance = NULL;

ATOM MyRegisterClass(HINSTANCE hInstance)
{
   WNDCLASSEX wcex;

   wcex.cbSize = sizeof(WNDCLASSEX);

   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MT5TICKCHART));
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
   wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MT5TICKCHART);
   wcex.lpszClassName = szWindowClass;
   wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

   return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND parentChart)
{
   hInst = hInstance; // Store instance handle in our global variable
   DWORD dwRemove = WS_MAXIMIZEBOX | WS_THICKFRAME;
   mainWindowHandle = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~dwRemove,
                                     CW_USEDEFAULT, CW_USEDEFAULT, 1120, 789, parentChart, NULL, hInstance, NULL);

   if (!mainWindowHandle)
   {
      return FALSE;
   }

   ShowWindow(mainWindowHandle, nCmdShow);
   UpdateWindow(mainWindowHandle);

   tickChartHWnd = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
                                  0, 0, 1100, 428, mainWindowHandle, (HMENU)NULL, hInstance, NULL);

   if (!tickChartHWnd)
   {
      MessageBox(tickChartHWnd, L"Cannot Create window", L"Error", MB_OK);
      return FALSE;
   }
   OldTickChartProc = SetWindowLongPtr(tickChartHWnd, GWLP_WNDPROC, (LONG_PTR)TickChartWndProc);
   ShowWindow(tickChartHWnd, nCmdShow);
   UpdateWindow(tickChartHWnd);

   barChartHWnd = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
                                 0, 427, 1100, 300, mainWindowHandle, (HMENU)NULL, hInstance, NULL);

   if (!barChartHWnd)
   {
      MessageBox(barChartHWnd, L"Cannot Create window", L"Error", MB_OK);
      return FALSE;
   }
   OldBarChartProc = SetWindowLongPtr(barChartHWnd, GWLP_WNDPROC, (LONG_PTR)BarChartWndProc);
   ShowWindow(barChartHWnd, nCmdShow);
   UpdateWindow(barChartHWnd);

   if (!LoadDataFile())
   {
      InitTickChartModuleStruct(mainWindowHandle);
   }
   mainMenu = GetMenu(mainWindowHandle);
   return TRUE;
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND - process the application menu
//  WM_PAINT   - Paint the main window
//  WM_DESTROY - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_CREATE:
   {
      toolBarHWnd = CreateDialog(hModuleInstance, MAKEINTRESOURCE(IDD_TOOLBAR), hWnd, ToolDlgProc);
      if (toolBarHWnd != NULL)
      {
         ShowWindow(toolBarHWnd, SW_SHOW);
         HWND editCntrl = GetDlgItem(toolBarHWnd, IDC_STEP_SIZE_EDT);
         if (editCntrl != NULL)
            OldEditControlProc = SetWindowLongPtr(editCntrl, GWLP_WNDPROC, (LONG_PTR)EditControlProc);
      }
      else
      {
         MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",
                    MB_OK | MB_ICONINFORMATION);
      }
   }
   break;
   case WM_COMMAND:
      if (MenuItemsActivationProc(hWnd, message, wParam, lParam))
         return DefWindowProc(hWnd, message, wParam, lParam);
      break;
   case UPDATE_CHARTS:
      chartModule.UpdateCharts((byte)wParam, lParam != 0);
      break;
   case REWIND_CHARTS:
      chartModule.StepBackwardBtnClicked(true, lParam != 0);
      break;
   case AUTO_SCROLL_UPDATE:
      chartModule.AutoScrollUpdate(lParam != 0);
      break;
   case WM_KEYDOWN:

      switch (wParam)
      {
      case 'm':
      case 'M':
         if (GetAsyncKeyState(VK_CONTROL))
         {
            RECT rect;
            GetWindowRect(hWnd, &rect);
            if (hideShowHMenu == NULL)
            {
               hideShowHMenu = GetMenu(hWnd);
               SetMenu(hWnd, NULL);
               MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top - 20, false);
            }
            else
            {
               SetMenu(hWnd, hideShowHMenu);
               MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top + 20, false);
               hideShowHMenu = NULL;
            }
         }
         break;
      }
      break;
   case WM_CLOSE:
      DestroyWindow(hWnd);
      break;
   case WM_DESTROY:
      if (timerHandle != 0)
      {
         timeKillEvent(timerHandle);
         timerHandle = 0;
      }
      DestroyWindow(toolBarHWnd);
      PostQuitMessage(0);
      break;
   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

INT_PTR CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   switch (Message)
   {
   case WM_COMMAND:
      ToolDlgFunctionsPanelProc(hwnd, Message, wParam, lParam);
      break;
   default:
      return (INT_PTR)FALSE;
   }
   return (INT_PTR)TRUE;
}
void ToolDlgFunctionsPanelProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   UINT tmp = 0;
   switch (LOWORD(wParam))
   {
   case IDC_LD_FROM_SERVER_BTN:
      chartModule.LoadFromServerBtnClicked();
      break;
   case IDC_LD_FROM_SERVER_PARTIAL_BTN:
      chartModule.LoadFromServerPartialBtnClicked();
      break;
   case IDC_CHART_AUTO_SCRL_CHBX:
      if (IsDlgButtonChecked(hwnd, IDC_CHART_AUTO_SCRL_CHBX))
      {
         appSets.chartAutoScroll = true;
      }
      else
      {
         appSets.chartAutoScroll = false;
      }
      chartModule.ChartAutoScrollChBxChanged();
      break;
   case IDC_STEP_FORWRD_BTN:
   {
      chartModule.StepForwardBtnClicked();
   }
   break;
   case IDC_STEP_BACKWRD_BTN:
   {
      chartModule.StepBackwardBtnClicked();
   }
   break;
   case IDC_MEASURE_CHBX:
      if (IsDlgButtonChecked(hwnd, IDC_MEASURE_CHBX))
         appSets.measurementTool = true;
      else
         appSets.measurementTool = false;
      chartModule.MeasureChBxChanged();

      break;
   case IDC_REPLAY_BTN:
      if (appSets.replayModeOn)
      {
         appSets.replayModeOn = false;
         SetDlgItemText(toolBarHWnd, IDC_REPLAY_BTN, L"Replay");
         if (timerHandle != 0)
         {
            timeKillEvent(timerHandle);
            timerHandle = 0;
         }
      }
      else
      {
         appSets.replayModeOn = true;
         SetDlgItemTextA(toolBarHWnd, IDC_REPLAY_BTN, "| |");
         int mode = TIME_PERIODIC;
         if (appSets.realTempo)
            mode = TIME_ONESHOT;

         timerHandle = timeSetEvent(appSets.timerInterval, 0, TimerFunction, 0, mode);
      }
      chartModule.ReplayBtnClicked();
      break;
   case IDC_SET_RANGE_BTN:
   {
      chartModule.SetRangeBtnClicked();
   }
   break;
   case IDC_SET_BAR_T_SIZE_BTN:
      chartModule.SetBarTickSizeBtnClicked();
      break;
   case IDC_MV_RANGE_UP_BTN:
   {
      chartModule.MoveRangeUpBtnClicked();
   }
   break;
   case IDC_MV_RANGE_DOWN_BTN:
   {
      chartModule.MoveRangeDownBtnClicked();
   }
   break;
   case IDC_LVLS_EDIT_CHBX:
      if (IsDlgButtonChecked(hwnd, IDC_LVLS_EDIT_CHBX))
      {
         EnableWindow(GetDlgItem(hwnd, IDC_LEVEL_ADD_BTN), true);
         EnableWindow(GetDlgItem(hwnd, IDC_LEVELS_DEL_BTN), true);
         PostMessage(MT5ParentChart, WM_LBUTTONUP, 0, -SELECT_SIGNED_LEVELS);
      }
      else
      {
         EnableWindow(GetDlgItem(hwnd, IDC_LEVEL_ADD_BTN), false);
         EnableWindow(GetDlgItem(hwnd, IDC_LEVELS_DEL_BTN), false);
         PostMessage(MT5ParentChart, WM_LBUTTONUP, 0, -UNSELECT_SIGNED_LEVELS);
      }
      chartModule.SignedLevelsEditChBxChanged();
      break;
   case IDC_LEVEL_ADD_BTN:
      chartModule.AddSignedLevelBtnClicked();
      break;
   case IDC_LEVELS_DEL_BTN:
      chartModule.DeleteSignedLevelsBtnClicked();
      break;
   }
}
bool MenuItemsActivationProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   int wmId = LOWORD(wParam);
   int tmp = 0;
   bool codeNotFound = false;
   switch (wmId)
   {
   case IDM_TICK_CHART_ZOOM_1:
   case IDM_TICK_CHART_ZOOM_2:
   case IDM_TICK_CHART_ZOOM_4:
   {
      CheckMenuItem(mainMenu, (UINT)(appSets.tickChartZoom / 2 + 200), MF_UNCHECKED);

      appSets.tickChartZoom = (wmId - 200) * 2;
      if (wmId == 200)
         appSets.tickChartZoom += 1;
      CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      chartModule.TickChartZoomChanged();
   }
   break;
   case IDM_BAR_CHART_CANDLE_WIDTH_4:
   case IDM_BAR_CHART_CANDLE_WIDTH_2:
   case IDM_BAR_CHART_CANDLE_WIDTH_1:
   {
      CheckMenuItem(mainMenu, (UINT)(appSets.barChartCandleWidth / 2 + 203), MF_UNCHECKED);

      appSets.barChartCandleWidth = (wmId - 203) * 2;
      if (wmId == 203)
         appSets.barChartCandleWidth += 1;
      CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      chartModule.BarChartCandleWidthChanged();
   }
   break;
   case IDM_TICK_TIME_SEP_0:
   case IDM_TICK_TIME_SEP_1:
   case IDM_TICK_TIME_SEP_5:
   case IDM_TICK_TIME_SEP_15:
   case IDM_TICK_TIME_SEP_30:
   case IDM_TICK_TIME_SEP_60:
   {
      switch (appSets.tickChartTimeSep)
      {
      case 0:
         tmp = IDM_TICK_TIME_SEP_0;
         break;
      case 1:
         tmp = IDM_TICK_TIME_SEP_1;
         break;
      case 5:
         tmp = IDM_TICK_TIME_SEP_5;
         break;
      case 15:
         tmp = IDM_TICK_TIME_SEP_15;
         break;
      case 30:
         tmp = IDM_TICK_TIME_SEP_30;
         break;
      case 60:
         tmp = IDM_TICK_TIME_SEP_60;
         break;
      }
      CheckMenuItem(mainMenu, (UINT)(tmp), MF_UNCHECKED);
      switch (wmId)
      {
      case IDM_TICK_TIME_SEP_0:
         tmp = 0;
         break;
      case IDM_TICK_TIME_SEP_1:
         tmp = 1;
         break;
      case IDM_TICK_TIME_SEP_5:
         tmp = 5;
         break;
      case IDM_TICK_TIME_SEP_15:
         tmp = 15;
         break;
      case IDM_TICK_TIME_SEP_30:
         tmp = 30;
         break;
      case IDM_TICK_TIME_SEP_60:
         tmp = 60;
         break;
      }
      appSets.tickChartTimeSep = tmp;
      CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      chartModule.TickChartTimeSepChanged();
   }
   break;
   case IDM_BAR_TIME_SEP_0:
   case IDM_BAR_TIME_SEP_1:
   case IDM_BAR_TIME_SEP_5:
   case IDM_BAR_TIME_SEP_15:
   case IDM_BAR_TIME_SEP_30:
   case IDM_BAR_TIME_SEP_60:
   {
      switch (appSets.barChartTimeSep)
      {
      case 0:
         tmp = IDM_BAR_TIME_SEP_0;
         break;
      case 1:
         tmp = IDM_BAR_TIME_SEP_1;
         break;
      case 5:
         tmp = IDM_BAR_TIME_SEP_5;
         break;
      case 15:
         tmp = IDM_BAR_TIME_SEP_15;
         break;
      case 30:
         tmp = IDM_BAR_TIME_SEP_30;
         break;
      case 60:
         tmp = IDM_BAR_TIME_SEP_60;
         break;
      }
      CheckMenuItem(mainMenu, (UINT)(tmp), MF_UNCHECKED);
      switch (wmId)
      {
      case IDM_BAR_TIME_SEP_0:
         tmp = 0;
         break;
      case IDM_BAR_TIME_SEP_1:
         tmp = 1;
         break;
      case IDM_BAR_TIME_SEP_5:
         tmp = 5;
         break;
      case IDM_BAR_TIME_SEP_15:
         tmp = 15;
         break;
      case IDM_BAR_TIME_SEP_30:
         tmp = 30;
         break;
      case IDM_BAR_TIME_SEP_60:
         tmp = 60;
         break;
      }
      appSets.barChartTimeSep = tmp;
      CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      chartModule.BarChartTimeSepChanged();
   }
   break;
   case IDM_FREEZE_RANGES:
      if (GetMenuState(mainMenu, IDM_FREEZE_RANGES, 0))
      {
         appSets.freezeRanges = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.freezeRanges = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.FreezeRanges();
      break;
   case IDM_RESET_CHARTS:
      chartModule.ResetCharts();
      break;
   case IDM_VIS_BID:
   {
      if (GetMenuState(mainMenu, IDM_VIS_BID, 0))
      {
         appSets.bidLineVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.bidLineVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.BidLineVisChanged();
   }
   break;
   case IDM_VIS_ASK:
   {
      if (GetMenuState(mainMenu, IDM_VIS_ASK, 0))
      {
         appSets.askLineVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.askLineVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.AskLineVisChanged();
   }
   break;
   case IDM_VIS_LEVELS:
      if (GetMenuState(mainMenu, IDM_VIS_LEVELS, 0))
      {
         appSets.signedLevelsVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.signedLevelsVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.SignedLevelsVisChanged();
      break;
   case IDM_VIS_AUTO_MOV_RANGE:
      if (GetMenuState(mainMenu, IDM_VIS_AUTO_MOV_RANGE, 0))
      {
         appSets.autoMovingRange = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.autoMovingRange = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.AutoMovingRangeChanged();
      break;
   case IDM_VIS_MPROFILE_ASK:
   {
      if (GetMenuState(mainMenu, IDM_VIS_MPROFILE_ASK, 0))
      {
         appSets.mProfileAskVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.mProfileAskVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.MProfileAskVisChanged();
   }
   break;
   case IDM_VIS_MPROFILE_BID:
   {
      if (GetMenuState(mainMenu, IDM_VIS_MPROFILE_BID, 0))
      {
         appSets.mProfileBidVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.mProfileBidVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.MProfileBidVisChanged();
   }
   break;
   case IDM_VIS_TICK_CHART:
   {
      if (GetMenuState(mainMenu, IDM_VIS_TICK_CHART, 0))
      {
         appSets.tickChartVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.tickChartVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.TickChartVisChanged();
   }
   break;
   case IDM_VIS_BAR_CHART:
   {
      if (GetMenuState(mainMenu, IDM_VIS_BAR_CHART, 0))
      {
         appSets.barChartVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.barChartVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.BarChartVisChanged();
   }
   break;
   case IDM_VIS_TIME_PARAMETER:
   {
      if (GetMenuState(mainMenu, IDM_VIS_TIME_PARAMETER, 0))
      {
         appSets.timeParamVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.timeParamVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.TimeParameterVisChanged();
   }
   break;
   case IDM_VIS_CUMULATIVE_ASK:
   {
      if (GetMenuState(mainMenu, IDM_VIS_CUMULATIVE_ASK, 0))
      {
         appSets.cumulativeAskVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.cumulativeAskVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.CumulativeAskVisChanged();
   }
   break;
   case IDM_VIS_CUMULATIVE_BID:
   {
      if (GetMenuState(mainMenu, IDM_VIS_CUMULATIVE_BID, 0))
      {
         appSets.cumulativeBidVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.cumulativeBidVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.CumulativeBidVisChanged();
   }
   break;
   case IDM_VIS_DISTANCE:
   {
      if (GetMenuState(mainMenu, IDM_VIS_DISTANCE, 0))
      {
         appSets.distanceVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.distanceVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.DistanceVisChanged();
   }
   break;
   case IDM_VIS_ROAD:
   {
      if (GetMenuState(mainMenu, IDM_VIS_ROAD, 0))
      {
         appSets.roadVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.roadVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.RoadVisChanged();
   }
   break;
   case IDM_VIS_TICKS:
   {
      if (GetMenuState(mainMenu, IDM_VIS_TICKS, 0))
      {
         appSets.ticksArrivedVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.ticksArrivedVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.TicksArrivedVisChanged();
   }
   break;
   case IDM_BAR_CHART_EVENTS:
   {
      if (GetMenuState(mainMenu, IDM_BAR_CHART_EVENTS, 0))
      {
         appSets.eventsOnBarChartVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.eventsOnBarChartVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.EventsOnBarChartVisChanged();
   }
   break;
   case IDM_TICK_CHART_EVENTS:
   {
      if (GetMenuState(mainMenu, IDM_TICK_CHART_EVENTS, 0))
      {
         appSets.eventsOnTickChartVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.eventsOnTickChartVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.EventsOnTickChartVisChanged();
   }
   break;
   case IDM_ORDERS:
      if (GetMenuState(mainMenu, IDM_ORDERS, 0))
      {
         appSets.ordersVis = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.ordersVis = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.OrdersVisChanged();
      break;
   case IDM_COLOR_TIME_PARAM:
   {
      if (GetMenuState(mainMenu, IDM_COLOR_TIME_PARAM, 0))
      {
         appSets.colorTimeParam = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.colorTimeParam = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.ColorTimeParamChanged();
   }
   break;
   case IDM_ZOOM_TIME_PARAM:
   {
      if (GetMenuState(mainMenu, IDM_ZOOM_TIME_PARAM, 0))
      {
         appSets.zoomTimeParam = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.zoomTimeParam = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.ZoomTimeParamChanged();
   }
   break;
   case IDM_SAVE_TICKS:
   {
      OPENFILENAME ofn;
      WCHAR szFileName[MAX_PATH] = L"";

      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
      ofn.hwndOwner = mainWindowHandle;
      ofn.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = szFileName;
      ofn.nMaxFile = MAX_PATH;
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
      ofn.lpstrDefExt = L"csv";

      if (GetSaveFileName(&ofn))
      {
         chartModule.SaveTicksClicked(ofn.lpstrFile);
      }
   }
   break;
   case IDM_PRINT_SCREEN:
   {
      OPENFILENAME ofn;
      WCHAR szFileName[MAX_PATH] = L"";

      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
      ofn.hwndOwner = mainWindowHandle;
      ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0";
      ofn.lpstrFile = szFileName;
      ofn.nMaxFile = MAX_PATH;
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
      ofn.lpstrDefExt = L"csv";

      if (GetSaveFileName(&ofn))
      {
         chartModule.PrintScreenClicked(ofn.lpstrFile);
      }
   }
   break;
   case IDM_TIMER_100:
   case IDM_TIMER_200:
   case IDM_TIMER_300:
   case IDM_TIMER_400:
   case IDM_TIMER_500:
   case IDM_TIMER_600:
   case IDM_TIMER_700:
   case IDM_TIMER_800:
   case IDM_TIMER_900:
   case IDM_TIMER_1000:
   case IDM_TIMER_2000:

      if (appSets.timerInterval == 2000)
         tmp = IDM_TIMER_2000;
      else
         tmp = appSets.timerInterval / 100 + 240;

      CheckMenuItem(mainMenu, (UINT)(tmp), MF_UNCHECKED);

      if (wmId == IDM_TIMER_2000)
         appSets.timerInterval = 2000;
      else
         appSets.timerInterval = (wmId - 240) * 100;

      CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      chartModule.TimerIntervalChanged();
      break;
   case IDM_PLAY_REAL_TEMPO:
      if (GetMenuState(mainMenu, IDM_PLAY_REAL_TEMPO, 0))
      {
         appSets.realTempo = false;
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
      }
      else
      {
         appSets.realTempo = true;
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
      }
      chartModule.RealTempoChanged();
      break;

   case ID_TOOLBAR_SHOW:
      if (GetMenuState(mainMenu, ID_TOOLBAR_SHOW, 0))
      {
         CheckMenuItem(mainMenu, wmId, MF_UNCHECKED);
         ShowWindow(toolBarHWnd, SW_HIDE);
      }
      else
      {
         CheckMenuItem(mainMenu, wmId, MF_CHECKED);
         ShowWindow(toolBarHWnd, SW_SHOW);
      }
      break;
   case IDM_EXIT:
      DestroyWindow(hwnd);
      break;
   default:
      codeNotFound = true;
   }
   return (codeNotFound);
}

// Message handler for about box.
LRESULT CALLBACK TickChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hdc;
   static int startPt = -1;

   switch (message)
   {
   case WM_MOUSELEAVE:
   {
      if (appSets.measurementTool)
      {
         tickChMouseTracking = false;
         SetWindowText(mainWindowHandle, szTitle);
      }
   }
   break;
   case WM_LBUTTONUP:
   case WM_LBUTTONDOWN:

   case WM_MOUSEMOVE:
   {
      if (appSets.measurementTool)
      {
         if (!tickChMouseTracking)
         {
            // Enable mouse tracking.
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = HOVER_DEFAULT;
            TrackMouseEvent(&tme);
            tickChMouseTracking = true;
         }

         POINT pt;
         GetCursorPos(&pt);
         ScreenToClient(hWnd, &pt);
         chartModule.TickChartTTipChanged(pt.x, pt.y, GetAsyncKeyState(VK_LBUTTON), szTitle);
      }
   }
   break;
   case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      chartModule.RefreshTickChartWindow((int)ps.rcPaint.left, (int)ps.rcPaint.top, (int)(ps.rcPaint.right - ps.rcPaint.left), (int)(ps.rcPaint.bottom - ps.rcPaint.top), (int)(ps.rcPaint.left), (int)(ps.rcPaint.top));

      EndPaint(hWnd, &ps);
      break;
   }
   return (CallWindowProc((WNDPROC)OldTickChartProc, hWnd, message, wParam, lParam));
}
LRESULT CALLBACK BarChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hdc;
   static int startPt = -1;

   switch (message)
   {
   case WM_MOUSELEAVE:
      if (appSets.measurementTool)
      {
         barChMouseTracking = false;
         SetWindowText(mainWindowHandle, szTitle);
      }
      break;
   case WM_LBUTTONUP:
   case WM_LBUTTONDOWN:
   case WM_MOUSEMOVE:
      if (appSets.measurementTool)
      {

         if (!tickChMouseTracking)
         {
            // Enable mouse tracking.
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = HOVER_DEFAULT;
            TrackMouseEvent(&tme);
            barChMouseTracking = true;
         }

         POINT pt;
         GetCursorPos(&pt);
         ScreenToClient(hWnd, &pt);
         chartModule.BarChartTTipChanged(pt.x, pt.y, GetAsyncKeyState(VK_LBUTTON), szTitle);
      }
      break;
   case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      chartModule.RefreshBarChartWindow((int)ps.rcPaint.left, (int)ps.rcPaint.top, (int)(ps.rcPaint.right - ps.rcPaint.left), (int)(ps.rcPaint.bottom - ps.rcPaint.top), (int)(ps.rcPaint.left), (int)(ps.rcPaint.top));
      EndPaint(hWnd, &ps);
      break;
   }
   return (CallWindowProc((WNDPROC)OldBarChartProc, hWnd, message, wParam, lParam));
}
LRESULT CALLBACK EditControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_KEYUP:
   {
      switch (wParam)
      {
      case VK_RETURN:
      {
         BOOL success;
         uint newValue = (uint)GetDlgItemInt(toolBarHWnd, IDC_STEP_SIZE_EDT, &success, false);
         if (success)
         {
            if (newValue > 0)
            {
               appSets.tickOffsetValue = newValue;
               SetDlgItemInt(toolBarHWnd, IDC_STEP_SIZE_EDT, newValue, false);
            }
         }
      }
      break;
      }
   }
   break;
   }
   return (CallWindowProc((WNDPROC)OldEditControlProc, hWnd, message, wParam, lParam));
}
void InitTickChartModuleStruct(HWND hwnd)
{
   ZeroMemory(&appSets, sizeof(appSets));

   appSets.askLineVis = true;
   appSets.autoMovingRange = true;
   appSets.barChartCandleWidth = 4;
   appSets.barChartTimeSep = 60;
   appSets.barChartVis = true;
   appSets.bidLineVis = true;
   appSets.colorTimeParam = false;
   appSets.cumulativeAskVis = false;
   appSets.cumulativeBidVis = false;
   appSets.distanceVis = false;
   appSets.eventsOnBarChartVis = false;
   appSets.eventsOnTickChartVis = false;
   appSets.freezeRanges = true;
   appSets.mProfileAskVis = false;
   appSets.mProfileBidVis = false;
   appSets.ordersVis = false;
   appSets.realTempo = false;
   appSets.roadVis = false;
   appSets.tickChartTimeSep = 5;
   appSets.tickChartVis = true;
   appSets.tickChartZoom = 2;
   appSets.ticksArrivedVis = false;
   appSets.timeParamVis = true;
   appSets.timerInterval = 1000;
   appSets.zoomTimeParam = false;
   appSets.chartAutoScroll = false;
   appSets.measurementTool = false;
   appSets.replayModeOn = false;
   appSets.tickOffsetValue = 50;
   appSets.barChartTickSize = 30;

   HMENU hm = GetMenu(hwnd);

   CheckMenuItem(hm, IDM_TICK_CHART_ZOOM_2, MF_CHECKED);
   CheckMenuItem(hm, IDM_BAR_CHART_CANDLE_WIDTH_4, MF_CHECKED);
   CheckMenuItem(hm, IDM_TICK_TIME_SEP_5, MF_CHECKED);
   CheckMenuItem(hm, IDM_BAR_TIME_SEP_60, MF_CHECKED);
   CheckMenuItem(hm, IDM_FREEZE_RANGES, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_ASK, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_BID, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_AUTO_MOV_RANGE, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_TICK_CHART, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_BAR_CHART, MF_CHECKED);
   CheckMenuItem(hm, IDM_VIS_TIME_PARAMETER, MF_CHECKED);
   CheckMenuItem(hm, IDM_TIMER_1000, MF_CHECKED);
   CheckMenuItem(hm, ID_TOOLBAR_SHOW, MF_CHECKED);

   SetDlgItemInt(toolBarHWnd, IDC_STEP_SIZE_EDT, appSets.tickOffsetValue, FALSE);
}
void CALLBACK TimerFunction(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
   // nextInterval in milliseconds
   int nextInterval = chartModule.OnTimer();
   if (nextInterval > 0)
      timerHandle = timeSetEvent(nextInterval, 0, TimerFunction, 0, TIME_ONESHOT);
}
int MainModuleProcedure(HWND parentWindow, HINSTANCE hInst, int cmdShow)
{
   hModuleInstance = hInst;
   MSG msg;
   hideShowHMenu = NULL;
   mainMenu = NULL;

   // Initialize global strings
   LoadString(hModuleInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
   LoadString(hModuleInstance, IDC_MT5TICKCHART, szWindowClass, MAX_LOADSTRING);
   MyRegisterClass(hModuleInstance);

   // Perform application initialization:
   if (!InitInstance(hModuleInstance, cmdShow))
   {
      return FALSE;
   }

   chartModule.PassSettingsObjRef(&appSets);
   RECT rect;
   GetWindowRect(tickChartHWnd, &rect);
   if (!chartModule.Initialize(MT5ParentChart, mainWindowHandle, tickChartHWnd, barChartHWnd, toolBarHWnd))
   {
      chartModule.ModuleDestroy();
      UnregisterClass(szWindowClass, NULL);
      return (0);
   }

   // Main message loop:
   while (GetMessage(&msg, NULL, 0, 0))
   {
      if (!IsDialogMessage(toolBarHWnd, &msg))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   chartModule.ModuleDestroy();
   UnregisterClass(szWindowClass, NULL);
   return (int)msg.wParam;
}
DWORD WINAPI MainModuleThreadFunction(LPVOID lpParam)
{
   hModuleInstance = (HINSTANCE)lpParam;
   MSG msg;
   hideShowHMenu = NULL;
   mainMenu = NULL;

   LARGE_INTEGER Frequency;
   QueryPerformanceFrequency(&Frequency);
   ProccessorFrequency = static_cast<long long>(Frequency.QuadPart);

   // Initialize global strings
   LoadString(hModuleInstance, IDC_MT5TICKCHART, szWindowClass, MAX_LOADSTRING);
   MyRegisterClass(hModuleInstance);

   // Perform application initialization:
   if (!InitInstance(hModuleInstance, SW_SHOWNORMAL, MT5ParentChart))
   {
      return FALSE;
   }

   HWND terminalWnd = GetAncestor(MT5ParentChart, 2u);
   LPARAM iconSmall = SendMessageA(terminalWnd, WM_GETICON, ICON_SMALL, 0);
   SendMessageA(mainWindowHandle, WM_SETICON, ICON_SMALL, iconSmall);
   LPARAM iconBig = SendMessageA(terminalWnd, WM_GETICON, ICON_BIG, 0);
   SendMessageA(mainWindowHandle, WM_SETICON, ICON_BIG, iconBig);

   chartModule.PassSettingsObjRef(&appSets);
   RECT rect;
   GetWindowRect(tickChartHWnd, &rect);
   if (!chartModule.Initialize(MT5ParentChart, mainWindowHandle, tickChartHWnd, barChartHWnd, toolBarHWnd))
   {
      chartModule.ModuleDestroy();
      UnregisterClass(szWindowClass, NULL);
      return (0);
   }
   PostMessage(MT5ParentChart, WM_LBUTTONUP, 0, -MODULE_INITIALIZED);
   // Main message loop:
   while (GetMessage(&msg, NULL, 0, 0))
   {
      if (!IsDialogMessage(toolBarHWnd, &msg))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   SaveDataFile();
   mainWindowHandle = NULL;
   threadHandle = NULL;
   chartModule.ModuleDestroy();
   UnregisterClass(szWindowClass, NULL);
   PostMessage(MT5ParentChart, WM_LBUTTONUP, 0, -CLOSE_CHART);
   return (int)msg.wParam;
}

bool SaveDataFile()
{
   HANDLE hFile;
   bool bSuccess = false;

   hFile = CreateFile(fullDataPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize = (DWORD)sizeof(TCHMODSET);
      DWORD dwWritten;

      if (WriteFile(hFile, &appSets, dwFileSize, &dwWritten, NULL))
      {
         bSuccess = true;
      }

      CloseHandle(hFile);
   }
   return (bSuccess);
}
bool LoadDataFile()
{
   HANDLE hFile;
   bool bSuccess = false;

   hFile = CreateFile(fullDataPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize;

      dwFileSize = GetFileSize(hFile, NULL);
      if (dwFileSize == sizeof(TICKCHARTMODULESETTINGS))
      {
         DWORD dwRead;

         if (ReadFile(hFile, &appSets, dwFileSize, &dwRead, NULL))
         {
            HMENU hm = GetMenu(mainWindowHandle);

            appSets.chartAutoScroll = true;

            UINT state = BST_CHECKED;
            if (!appSets.chartAutoScroll)
               state = BST_UNCHECKED;

            CheckDlgButton(toolBarHWnd, IDC_CHART_AUTO_SCRL_CHBX, state);
            SetDlgItemInt(toolBarHWnd, IDC_STEP_SIZE_EDT, appSets.tickOffsetValue, FALSE);

            switch (appSets.tickChartZoom)
            {
            case 4:
               CheckMenuItem(hm, IDM_TICK_CHART_ZOOM_4, MF_CHECKED);
               break;
            case 2:
               CheckMenuItem(hm, IDM_TICK_CHART_ZOOM_2, MF_CHECKED);
               break;
            case 1:
               CheckMenuItem(hm, IDM_TICK_CHART_ZOOM_1, MF_CHECKED);
               break;
            }

            switch (appSets.barChartCandleWidth)
            {
            case 4:
               CheckMenuItem(hm, IDM_BAR_CHART_CANDLE_WIDTH_4, MF_CHECKED);
               break;
            case 2:
               CheckMenuItem(hm, IDM_BAR_CHART_CANDLE_WIDTH_2, MF_CHECKED);
               break;
            case 1:
               CheckMenuItem(hm, IDM_BAR_CHART_CANDLE_WIDTH_1, MF_CHECKED);
               break;
            }

            switch (appSets.tickChartTimeSep)
            {
            case 0:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_0, MF_CHECKED);
               break;
            case 1:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_1, MF_CHECKED);
               break;
            case 5:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_5, MF_CHECKED);
               break;
            case 15:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_15, MF_CHECKED);
               break;
            case 30:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_30, MF_CHECKED);
               break;
            case 60:
               CheckMenuItem(hm, IDM_TICK_TIME_SEP_60, MF_CHECKED);
               break;
            }

            switch (appSets.barChartTimeSep)
            {
            case 0:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_0, MF_CHECKED);
               break;
            case 1:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_1, MF_CHECKED);
               break;
            case 5:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_5, MF_CHECKED);
               break;
            case 15:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_15, MF_CHECKED);
               break;
            case 30:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_30, MF_CHECKED);
               break;
            case 60:
               CheckMenuItem(hm, IDM_BAR_TIME_SEP_60, MF_CHECKED);
               break;
            }

            state = MF_CHECKED;
            if (!appSets.freezeRanges)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_FREEZE_RANGES, state);

            state = MF_CHECKED;
            if (!appSets.askLineVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_ASK, state);

            state = MF_CHECKED;
            if (!appSets.bidLineVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_BID, state);

            state = MF_CHECKED;
            if (!appSets.signedLevelsVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_LEVELS, state);

            state = MF_CHECKED;
            if (!appSets.autoMovingRange)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_AUTO_MOV_RANGE, state);

            state = MF_CHECKED;
            if (!appSets.tickChartVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_TICK_CHART, state);

            state = MF_CHECKED;
            if (!appSets.barChartVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_BAR_CHART, state);

            state = MF_CHECKED;
            if (!appSets.timeParamVis)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_VIS_TIME_PARAMETER, state);

            state = MF_UNCHECKED;
            if (appSets.zoomTimeParam)
               state = MF_CHECKED;
            CheckMenuItem(hm, IDM_ZOOM_TIME_PARAM, state);

            switch (appSets.timerInterval)
            {
            case 2000:
               CheckMenuItem(hm, IDM_TIMER_2000, MF_CHECKED);
               break;
            case 1000:
               CheckMenuItem(hm, IDM_TIMER_1000, MF_CHECKED);
               break;
            case 900:
               CheckMenuItem(hm, IDM_TIMER_900, MF_CHECKED);
               break;
            case 800:
               CheckMenuItem(hm, IDM_TIMER_800, MF_CHECKED);
               break;
            case 700:
               CheckMenuItem(hm, IDM_TIMER_700, MF_CHECKED);
               break;
            case 600:
               CheckMenuItem(hm, IDM_TIMER_600, MF_CHECKED);
               break;
            case 500:
               CheckMenuItem(hm, IDM_TIMER_500, MF_CHECKED);
               break;
            case 400:
               CheckMenuItem(hm, IDM_TIMER_400, MF_CHECKED);
               break;
            case 300:
               CheckMenuItem(hm, IDM_TIMER_300, MF_CHECKED);
               break;
            case 200:
               CheckMenuItem(hm, IDM_TIMER_200, MF_CHECKED);
               break;
            case 100:
               CheckMenuItem(hm, IDM_TIMER_100, MF_CHECKED);
               break;
            }

            CheckMenuItem(hm, ID_TOOLBAR_SHOW, MF_CHECKED);

            state = MF_CHECKED;
            if (!appSets.realTempo)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_PLAY_REAL_TEMPO, state);

            state = MF_CHECKED;
            if (!appSets.colorTimeParam)
               state = MF_UNCHECKED;
            CheckMenuItem(hm, IDM_COLOR_TIME_PARAM, state);

            appSets.distanceVis = false;
            appSets.roadVis = false;
            appSets.ticksArrivedVis = false;
            appSets.eventsOnBarChartVis = false;
            appSets.eventsOnTickChartVis = false;

            appSets.cumulativeAskVis = false;
            appSets.cumulativeBidVis = false;

            appSets.mProfileAskVis = false;
            appSets.mProfileBidVis = false;
            appSets.measurementTool = false;

            bSuccess = true;
         }
      }
      CloseHandle(hFile);
   }
   return (bSuccess);
}