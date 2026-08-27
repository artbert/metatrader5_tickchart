#pragma once
#include "canvas.h"
#include <algorithm>
#include <cfloat>
#include <cstring>

enum ENUM_SHOW_FLAGS
{
   FLAG_SHOW_NONE = 0,
   FLAG_SHOW_LEGEND = 1,
   FLAG_SHOW_SCALE_LEFT = 2,
   FLAG_SHOW_SCALE_RIGHT = 4,
   FLAG_SHOW_SCALE_TOP = 8,
   FLAG_SHOW_SCALE_BOTTOM = 16,
   FLAG_SHOW_GRID = 32,
   FLAG_SHOW_DESCRIPTORS = 64,
   FLAG_SHOW_VALUE = 128,
   FLAG_SHOW_PERCENT = 256,
   FLAGS_SHOW_SCALES = (FLAG_SHOW_SCALE_LEFT + FLAG_SHOW_SCALE_RIGHT +
                        FLAG_SHOW_SCALE_TOP + FLAG_SHOW_SCALE_BOTTOM),
   FLAGS_SHOW_ALL = (FLAG_SHOW_LEGEND + FLAGS_SHOW_SCALES + FLAG_SHOW_GRID +
                     FLAG_SHOW_DESCRIPTORS + FLAG_SHOW_VALUE + FLAG_SHOW_PERCENT)
};

#define IS_SHOW_LEGEND ((m_show_flags & FLAG_SHOW_LEGEND) != 0)
#define IS_SHOW_SCALES ((m_show_flags & FLAGS_SHOW_SCALES) != 0)
#define IS_SHOW_SCALE_LEFT ((m_show_flags & FLAG_SHOW_SCALE_LEFT) != 0)
#define IS_SHOW_SCALE_RIGHT ((m_show_flags & FLAG_SHOW_SCALE_RIGHT) != 0)
#define IS_SHOW_SCALE_TOP ((m_show_flags & FLAG_SHOW_SCALE_TOP) != 0)
#define IS_SHOW_SCALE_BOTTOM ((m_show_flags & FLAG_SHOW_SCALE_BOTTOM) != 0)
#define IS_SHOW_GRID ((m_show_flags & FLAG_SHOW_GRID) != 0)
#define IS_SHOW_DESCRIPTORS ((m_show_flags & FLAG_SHOW_DESCRIPTORS) != 0)
#define IS_SHOW_VALUE ((m_show_flags & FLAG_SHOW_VALUE) != 0)
#define IS_SHOW_PERCENT ((m_show_flags & FLAG_SHOW_PERCENT) != 0)

struct CalendarEvent
{
   time_t eventDateTime;
   char eventSymbol[20];
   byte importance;
   char eventDescription[200];
   short eventChange;
};

class ChartCanvas : public Canvas
{
protected:
   uint m_color_background;
   uint m_color_border;
   uint m_color_text;
   uint m_color_grid;

   uint m_show_flags;

   bool vScaleParamsChanged;

   RECT m_data_area;
   int m_y_min;
   int m_y_max;
   double m_v_scale_min;
   double m_v_scale_max;
   int m_y_0;
   double m_scale_y;
   int m_dy_grid;
   uint m_num_grid;
   char (*m_scale_text)[50];
   int m_scale_text_size;
   int m_scale_digits;
   uint m_data_total;
   uint m_allowed_show_flags;

   BLENDFUNCTION blendFunctionParams;
   HDC alphaBlendHDc;
   HBITMAP oldBmp;

public:
   ChartCanvas(void);
   ~ChartCanvas(void);

   virtual bool Create(HWND hWnd, const int width, const int height);
   void ShowFlags(const uint flags, const bool redraw = true);

   void VScaleParams(const double max, const double min, const uint grid, const bool redraw = true);
   void ShowScaleTop(const bool flag, const bool redraw = true);
   void ShowScaleRight(const bool flag, const bool redraw = true);
   void ShowScaleLeft(const bool flag, const bool redraw = true);
   void ShowScaleBottom(const bool flag, const bool redraw = true);
   void ShowLegend(const bool flag, const bool redraw = true);
   void ShowGrid(const bool flag = true, const bool redraw = true);
   void SetScaleDigits(const int digits)
   {
      m_scale_digits = digits;
   }
   void ShowDescriptors(const bool flag = true, const bool redraw = true);
   void ColorBackground(const uint value, const bool redraw = true);
   void ColorBorder(const uint value, const bool redraw = true);
   void ColorText(const uint value, const bool redraw = true);
   void ColorGrid(const uint value, const bool redraw = true)
   {
      m_color_grid = value;
   }
   uint ColorBackground(void) const
   {
      return (m_color_background);
   }
   uint ColorBorder(void) const
   {
      return (m_color_border);
   }
   uint ColorText(void) const
   {
      return (m_color_text);
   }
   uint ColorGrid(void) const
   {
      return (m_color_grid);
   }
   double VScaleMin(void) const
   {
      return (m_v_scale_min);
   }
   double VScaleMax(void) const
   {
      return (m_v_scale_max);
   }

   void PrepareAlphaBlend(HWND hWnd);
   void ReleaseAlphaBlend();
   void ApplyAlphaBlend(HWND hWnd, int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);

protected:
   virtual void Redraw();
   virtual void Redraw(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
   virtual void DrawBackground(void);
   virtual void DrawScales(void);
   virtual void CalcScales(void);
   virtual int DrawScaleTop(const bool draw = true);
   virtual int DrawScaleBottom(const bool draw = true);
   virtual int DrawScaleLeft(const bool draw = true);
   virtual int DrawScaleRight(const bool draw = true);
   virtual void DrawGrid(void);
   virtual void DrawChart(void);
   virtual void DrawData(const uint idx = 0) {}
};