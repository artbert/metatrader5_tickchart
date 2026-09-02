#pragma once
#include "canvas.hpp"
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

   RECT m_data_area{};
   int m_y_min{};
   int m_y_max{};
   double m_v_scale_min;
   double m_v_scale_max;
   int m_y_0{};
   double m_scale_y{};
   int m_dy_grid{};
   uint m_num_grid;
   char (*m_scale_text)[50];
   int m_scale_text_size;
   int m_scale_digits;
   uint m_data_total;
   uint m_allowed_show_flags;

   BLENDFUNCTION blendFunctionParams{};
   HDC alphaBlendHDc;
   HBITMAP oldBmp;

public:
   ChartCanvas();
   ~ChartCanvas();

   bool Create(HWND hWnd, int width, int height) override;
   void ShowFlags(uint flags, bool redraw = true);

   void VScaleParams(double max, double min, uint grid, bool redraw = true);
   void ShowScaleTop(bool flag, bool redraw = true);
   void ShowScaleRight(bool flag, bool redraw = true);
   void ShowScaleLeft(bool flag, bool redraw = true);
   void ShowScaleBottom(bool flag, bool redraw = true);
   void ShowLegend(bool flag, bool redraw = true);
   void ShowGrid(bool flag = true, bool redraw = true);
   void SetScaleDigits(const int digits)
   {
      m_scale_digits = digits;
   }
   void ShowDescriptors(bool flag = true, bool redraw = true);
   void ColorBackground(uint value, bool redraw = true);
   void ColorBorder(uint value, bool redraw = true);
   void ColorText(uint value, bool redraw = true);
   void ColorGrid(const uint value, const bool redraw = true)
   {
      m_color_grid = value;
   }
   [[nodiscard]] uint ColorBackground() const
   {
      return (m_color_background);
   }
   [[nodiscard]] uint ColorBorder() const
   {
      return (m_color_border);
   }
   [[nodiscard]] uint ColorText() const
   {
      return (m_color_text);
   }
   [[nodiscard]] uint ColorGrid() const
   {
      return (m_color_grid);
   }
   [[nodiscard]] double VScaleMin() const
   {
      return (m_v_scale_min);
   }
   [[nodiscard]] double VScaleMax() const
   {
      return (m_v_scale_max);
   }

   void PrepareAlphaBlend(HWND hWnd);
   void ReleaseAlphaBlend();
   void ApplyAlphaBlend(HWND hWnd, int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);

protected:
   virtual void Redraw();
   virtual void Redraw(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
   virtual void DrawBackground();
   virtual void DrawScales();
   virtual void CalcScales();
   virtual int DrawScaleTop(bool draw = true);
   virtual int DrawScaleBottom(bool draw = true);
   virtual int DrawScaleLeft(bool draw = true);
   virtual int DrawScaleRight(bool draw = true);
   virtual void DrawGrid();
   virtual void DrawChart();
   virtual void DrawData(const uint idx = 0) {}
};