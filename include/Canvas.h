#pragma once
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long long ulong;

#define XRGB(r, g, b) (0xFF000000 | (byte(r) << 16) | (byte(g) << 8) | byte(b))
#define XRGB_gdi(r, g, b) ((byte(r) << 16) | (byte(g) << 8) | byte(b))

static double POWER_OF_10[16] =
    {
        1.0, 10.0, 100.0, 1000.0, 10000.0,
        100000.0, 1000000.0, 10000000.0, 100000000.0,
        1000000000.0, 10000000000.0, 100000000000.0,
        1000000000000.0, 10000000000000.0, 100000000000000.0, 1000000000000000.0};

inline double NormalizeDouble(double x, uint places)
{
   double powTen;
   if (places < 16)
      powTen = POWER_OF_10[places];
   else
      powTen = POWER_OF_10[15];

   return (floor((x * powTen) + 0.5) / powTen);
}
extern WCHAR TIMETABLE[60][3];
extern WCHAR WNUMBERS[10][2];
extern char TIMETABLE_A[60][3];
extern char WNUMBERS_A[10][2];

template <class _cls>
inline _cls MaxInArray(_cls arr[], int start, int count)
{
   _cls result = arr[start];
   for (int i = start + 1; i < start + count; i++)
   {
      if (arr[i] > result)
      {
         result = arr[i];
      }
   }
   return (result);
}
template <class _cls>
inline void MinMax(_cls arr[], int start, int count, _cls &minOut, _cls &maxOut)
{
   _cls mn = arr[start];
   _cls mx = arr[start];
   for (int i = start + 1; i < start + count; i++)
   {
      if (arr[i] > mx)
         mx = arr[i];
      else if (arr[i] < mn)
         mn = arr[i];
   }
   minOut = mn;
   maxOut = mx;
}
template <class _cls>
inline _cls MinInArray(_cls arr[], int start, int count)
{
   _cls result = arr[start];
   for (int i = start + 1; i < start + count; i++)
   {
      if (arr[i] < result)
      {
         result = arr[i];
      }
   }
   return (result);
}
class Canvas
{
private:
   HBITMAP hbm;
   HBITMAP oldBitmap;
   BITMAPINFO bmpInfo;
   HDC hDCMem;
   HFONT hf_12, hf_09, hf_07;
   HFONT olfFont;
   HWND canvasWindow;

   int lettersData[41][2][77]; //[letter index][0-ind,1-color]
   int lettersBitIndsUsed[41];

   int lettersData_09[41][2][40]; //[letter index][0-ind,1-color]
   int lettersBitIndsUsed_09[41];

   bool initialized;

protected:
   int m_width;  // canvas width
   int m_height; // canvas height
   //--- for text
   wchar_t m_fontname[100]; // font name
   int m_fontsize;          // font size
   uint m_fontflags;        // font flags
   uint m_fontangle;        // angle of text tilt to the X axis in 0.1 degrees
   //--- data
   uint *m_pixels; // array of pixels
public:
   Canvas(void);
   ~Canvas(void);
   void ArrayFill(int start, int count, int val);
   void Erase(const uint clr = 0);
   void PixelSet(const int x, const int y, const uint clr);
   void LineVertical(int x, int y1, int y2, const uint clr);
   void SafeSortedLineVertical(int x, int y1, int y2, const uint clr);
   void LineVerticalDott(int x, int y1, int y2, const uint clr);
   void LineHorizontal(int x1, int x2, int y, const uint clr);
   void SafeSortedLineHorizontal(int x1, int x2, int y, const uint clr);
   void LineHorizontalDott(int x1, int x2, int y, const uint clr);
   void SafeSortedLineHorizontalDott(int x1, int x2, int y, const uint clr);
   void Line(int x1, int y1, int x2, int y2, const uint clr);
   void Rectangle(int x1, int y1, int x2, int y2, const uint clr);
   void SafeSortedRectangle(int x1, int y1, int x2, int y2, const uint clr);
   void FillRectangle(int x1, int y1, int x2, int y2, const uint clr);
   void SafeSortedFillRectangle(int x1, int y1, int x2, int y2, const uint clr);
   void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const uint clr);
   void FillCircle(int x, int y, int r, const uint clr);
   bool SelectFont(int size);
   void DrawTextOut_A(LPRECT lprc, LPCSTR text, uint alignment = 0);
   void TextColorSet(const uint clr);

   void DrawBitTimeSepStamp_09(const char arrayOfSigns[], int arrSize, int x, int y, int leftLimit, int rightLimit);
   void DrawBitText_12(const char arrayOfSigns[], int arrSize, int x, int y, bool safeMode = true, int leftLimit = -1, int rightLimit = -1);

   int Width() const
   {
      return (m_width);
   }
   int Height() const
   {
      return (m_height);
   }

   virtual bool Create(HWND hWnd, const int width, const int height);
   bool Destroy();
   void Update();
   void Update(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc);
   int TextWidth_A(LPCSTR text);
   void CreatePixelFontSet(uint eraseColor);

private:
};