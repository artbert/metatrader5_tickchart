#include "StdAfx.h"
#include "ChartCanvas.h"


ChartCanvas::ChartCanvas():
   m_color_background(XRGB_gdi(0xFF,0xFF,0xFF)),
   m_color_border(XRGB_gdi(0x9F,0x9F,0x9F)),
   m_color_text(XRGB_gdi(0x3F,0x3F,0x3F)),
   m_color_grid(XRGB_gdi(0xCF,0xCF,0xCF)),
   m_allowed_show_flags(FLAGS_SHOW_ALL),
   m_show_flags(FLAG_SHOW_NONE),
   m_data_total(0),
   m_v_scale_min(0.0),
   m_v_scale_max(10.0),
   m_num_grid(5),
   m_scale_digits(0),
   m_scale_text(NULL),
   m_scale_text_size(0)
{
   alphaBlendHDc=NULL;
   oldBmp=NULL;
   blendFunctionParams.BlendOp = AC_SRC_OVER;
   blendFunctionParams.BlendFlags = 0;
   blendFunctionParams.AlphaFormat = 0;   // use source alpha
   blendFunctionParams.SourceConstantAlpha = 50;   // use constant alpha, with
   vScaleParamsChanged=true;
}


ChartCanvas::~ChartCanvas()
{
   if(m_scale_text!=NULL)
      delete[] m_scale_text;
}

bool ChartCanvas::Create(HWND hWnd,const int width,const int height)
{
   m_color_background=XRGB_gdi(0xFF,0xFF,0xFF);
   m_color_border=XRGB_gdi(0x9F,0x9F,0x9F);
   m_color_text=XRGB_gdi(0x3F,0x3F,0x3F);
   m_color_grid=XRGB_gdi(0xCF,0xCF,0xCF);
   m_allowed_show_flags=FLAGS_SHOW_ALL;
   m_show_flags=FLAG_SHOW_NONE;
   m_v_scale_min=0.0;
   m_v_scale_max=10.0;
   m_num_grid=5;
   m_scale_digits=0;

   if(!Canvas::Create(hWnd,width,height))
   {
      return(false);
   }
   m_data_total=1;



   return(true);
}
void ChartCanvas::PrepareAlphaBlend(HWND hWnd)
{
   if(alphaBlendHDc!=NULL)
      DeleteDC(alphaBlendHDc);

   HDC hDC = GetDC(hWnd);
   alphaBlendHDc=CreateCompatibleDC(hDC);

   HBITMAP hBmp = CreateCompatibleBitmap(hDC,m_width,m_height);
   oldBmp = (HBITMAP)SelectObject(alphaBlendHDc,hBmp);
   HBRUSH m_brush = CreateSolidBrush(m_color_text);

   RECT rct;
   rct.left=0;
   rct.top=0;
   rct.right=m_width-1;
   rct.bottom=m_height-1;

   FillRect(alphaBlendHDc,&rct,m_brush);

   DeleteObject(m_brush);
   ReleaseDC(hWnd, hDC);
}
void ChartCanvas::ReleaseAlphaBlend()
{
   DeleteObject(SelectObject(alphaBlendHDc,oldBmp));
   DeleteDC(alphaBlendHDc);
   alphaBlendHDc=NULL;
}
void ChartCanvas::ApplyAlphaBlend(HWND hWnd,int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   if(alphaBlendHDc!=NULL)
   {
      HDC hDC = GetDC(hWnd);
      AlphaBlend(hDC, nXDest, nYDest, nWidth, nHeight, alphaBlendHDc, nXSrc, nYSrc, nWidth, nHeight, blendFunctionParams);
      ReleaseDC(hWnd, hDC);
   }
}
void ChartCanvas::Redraw()
{
   //LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
   //LARGE_INTEGER Frequency;
   //QueryPerformanceFrequency(&Frequency);
   //int static offff=50;
   //QueryPerformanceCounter(&StartingTime);
   bool updateWhole=false;

   int gap=m_fontsize/(-10);

   if(vScaleParamsChanged)
   {
      updateWhole=true;
      m_data_area.left=gap;
      m_data_area.top=gap;
      m_data_area.right=m_width-gap;
      m_data_area.bottom=m_height-gap;
   }

   //ValuesCheck();
   DrawBackground();


   //
////--- draw auxiliary elements
//    if(IS_SHOW_LEGEND)
//        DrawLegend();

   if(IS_SHOW_SCALES && vScaleParamsChanged)
      DrawScales();

   if(IS_SHOW_GRID)
      DrawGrid();

   if(IS_SHOW_DESCRIPTORS)
      DrawDescriptors();



   DrawChart();


   //offff=50;
   //Update();
   //}
   //else
   //{

   //int len=(m_data_area.right-m_data_area.left)-10;
   //int offst=m_data_area.left+1;
   //for(int i=m_height-2; i--;)
   //{
   //   offst+=m_width;
   //ArrayFill(offst,len,m_color_background);
   //memmove(&m_pixels[offst],&m_pixels[offst+1],(len-1)*4);
   //   memcpy(&m_pixels[offst],&m_pixels[offst+1],(len-1)*4);
   //}
   //offff+=5;
   //Update(100,100,500,200,101,101);
   //}

   if(updateWhole)
      Update();
   else
      Update(m_data_area.left+1,1,(m_data_area.right-m_data_area.left)-10,m_height-2,m_data_area.left+1,1);
   ////Update(m_data_area.left+100,50,500,100,m_data_area.left+100,50);
   //QueryPerformanceCounter(&EndingTime);

   //ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
   //long long interval = static_cast<long long>(ElapsedMicroseconds.QuadPart*1000000 / (Frequency.QuadPart));

   //static double totalSum_1=0,divider_1=1,result_1=0,totalSum_2=0,divider_2=1,result_2=0;
   //long long r =0;
   //if(m_height>400)
   //{
   //   totalSum_1+=(double)interval;
   //   result_1=totalSum_1/divider_1;
   //   divider_1++;
   //   r = static_cast<long long>(result_1);
   //}
   //else
   //{
   //   totalSum_2+=(double)interval;
   //   result_2=totalSum_2/divider_2;
   //   divider_2++;
   //   r = static_cast<long long>(result_2);
   //}

   //char text[1000];
   //_i64toa_s(r,text,1000,10);
   //DrawBitText_12(text,5,m_data_area.left+6,100,false);
   ////_i64toa_s(max,text,1000,10);
   ////DrawBitText_12(text,15,m_data_area.left+6,108,false);
   ////Update();
   //if(updateWhole)
   //   Update();
   //else
   //   Update(m_data_area.left+1,1,(m_data_area.right-m_data_area.left)-10,m_height-2,m_data_area.left+1,1);
}
void ChartCanvas::Redraw(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   int gap=m_fontsize/(-10);
   if(vScaleParamsChanged)
   {
      m_data_area.left=gap;
      m_data_area.top=gap;
      m_data_area.right=m_width-gap;
      m_data_area.bottom=m_height-gap;
   }

   //ValuesCheck();
   DrawBackground();
////--- draw auxiliary elements
//    if(IS_SHOW_LEGEND)
//        DrawLegend();
   if(IS_SHOW_SCALES && vScaleParamsChanged)
      //if(IS_SHOW_SCALES)
      DrawScales();
   if(IS_SHOW_GRID)
      DrawGrid();
   if(IS_SHOW_DESCRIPTORS)
      DrawDescriptors();
//--- draw data
   DrawChart();
//--- fix changes
   Update(nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc);
}
void ChartCanvas::ValuesCheck(void)
{
}
//'memset' can bu used in case 'm_color_background' is gray color,
//or can be casted to unsigned char
void ChartCanvas::DrawBackground(void)
{
   //To use memset, m_color_background must fit unsigned char value (int parameter casted to unsigned char in memset)
   if(vScaleParamsChanged)
   {
      Erase(m_color_background);
      Rectangle(0,0,m_width-1,m_height-1,m_color_border);
   }
   else
   {
      int len=(m_data_area.right-m_data_area.left)-10;
      //int offst=m_data_area.left+1;
      uint *offst = &m_pixels[m_data_area.left+1];
      for(int i=m_height-2; i--;)
      {
         offst+=m_width;
         //ArrayFill(offst,len,m_color_background);
         memset(offst,m_color_background,len*4);
      }
   }
}
void ChartCanvas::DrawScales(void)
{
//--- recalculate
   CalcScales();
//--- redraw scales
   if(IS_SHOW_SCALE_LEFT)
      DrawScaleLeft();
   if(IS_SHOW_SCALE_RIGHT)
      DrawScaleRight();
   if(IS_SHOW_SCALE_TOP)
      DrawScaleTop();
   if(IS_SHOW_SCALE_BOTTOM)
      DrawScaleBottom();
}
void ChartCanvas::CalcScales(void)
{
   int width =m_data_area.right-m_data_area.left;
   int height=m_data_area.bottom-m_data_area.top;
//--- limits
   m_y_max=m_data_area.top+DrawScaleTop(false);
   m_y_min=m_data_area.bottom-DrawScaleBottom(false);
//--- additional
   m_dy_grid=(int)((m_y_min-m_y_max)/m_num_grid);
   m_y_max+=(int)(((m_y_min-m_y_max)-m_dy_grid*m_num_grid)/2);
   m_y_min=(int)(m_y_max+m_dy_grid*m_num_grid);
//--- normalize
   if(m_v_scale_min>=0.0)
      m_y_0=m_y_min;
   else
   {
      if(m_v_scale_max<=0.0)
         m_y_0=m_y_max;
      else
         m_y_0=(int)(m_y_max+(m_y_min-m_y_max)*m_v_scale_max/(m_v_scale_max-m_v_scale_min));
   }
//--- scale
   m_scale_y=(m_v_scale_max!=m_v_scale_min) ?(m_y_min-m_y_max)/(m_v_scale_max-m_v_scale_min) : 1;
//--- labels on scale
   if(m_scale_text_size!=m_num_grid+1)
   {
      if(m_scale_text!=NULL)
         delete[] m_scale_text;
      m_scale_text = new char[m_num_grid+1][50];
      m_scale_text_size=m_num_grid+1;
   }
   double val=m_v_scale_min;
   double dval=(m_v_scale_max-m_v_scale_min)/m_num_grid;

   for(uint i=0; i<=m_num_grid; i++,val+=dval)
   {
      //swprintf(m_scale_text[i], L"%.*f",m_scale_digits, val);
      sprintf_s(m_scale_text[i],"%.*f",m_scale_digits, val);
   }
}
int ChartCanvas::DrawScaleTop(const bool draw)
{
   int size=0;
   if(!IS_SHOW_SCALE_TOP)
      return(0);
   if(draw)
   {
      Line(m_data_area.left,m_y_max,m_data_area.right,m_y_max,m_color_text);
   }
   return(size);
}
int ChartCanvas::DrawScaleBottom(const bool draw)
{
   int size=0;
   if(!IS_SHOW_SCALE_BOTTOM)
      return(0);
   size=10;
   if(draw)
   {
      m_data_area.bottom-=10;
      Line(m_data_area.left,m_data_area.bottom,m_data_area.right,m_data_area.bottom,m_color_text);
   }
   return(size);
}
int ChartCanvas::DrawScaleLeft(const bool draw)
{
//--- check flag
   if(!IS_SHOW_SCALE_LEFT)
      return(0);
//--- variables
   int x1=m_data_area.left;
   int x2;
   int y=m_y_min;
//--- calculate scale width
   int size=0;
   for(uint i=0; i<=m_num_grid; i++)
   {
      /* if(size<TextWidth_A(m_scale_text[i]))
         size=TextWidth_A(m_scale_text[i]);*/
      int j=0;
      for(; j<50; j++)
      {
         if(m_scale_text[i][j]==NULL)
            break;
      }
      if(size<j)
         size=j;
   }
//--- add indent and graduation mark (for now 5 pixels)
   //size+=5+5;
   size=size*7+10;
//--- draw
   if(draw)
   {
      x2=x1+size;
      x1=x2-5;
      //--- draw line
      //Line(x2,y,x2,m_y_max,m_color_text);
      SafeSortedLineVertical(x2,m_y_max,y,m_color_text);
      //--- set font
      //FontAngleSet(0);
      //string oldFont=m_fontname;
      //m_fontname="Consolas";
      //--- draw text
      //RECT rct;
      //rct.right=x1-5;
      for(uint i=0; i<=m_num_grid; i++,y-=m_dy_grid)
      {
         //Line(x1,y,x2,y,m_color_text);
         SafeSortedLineHorizontal(x1,x2,y,m_color_text);
         if((int)i<m_scale_text_size)
         {
            //rct.top=y;
            //rct.bottom=y;
            //DrawTextOut_A(&rct,m_scale_text[i],DT_SINGLELINE|DT_NOCLIP|DT_RIGHT|DT_VCENTER);
            DrawBitText_12(m_scale_text[i],10,m_data_area.left,y-4,false);
         }
      }
      //m_fontname=oldFont;
      //--- adjust data area
      m_data_area.left+=size;
   }
//--- return width
   return(size);
}
int ChartCanvas::DrawScaleRight(const bool draw)
{
//--- check flag
   if(!IS_SHOW_SCALE_RIGHT)
      return(0);
//--- variables
   int x1;
   int x2=m_data_area.right;
   int y =m_y_min;
//--- calculate scale width
   int size=0;
   for(uint i=0; i<=m_num_grid; i++)
   {
      if(size<TextWidth_A(m_scale_text[i]))
         size=TextWidth_A(m_scale_text[i]);
   }
//--- add indent and graduation mark (for now 5 pixels)
   size+=5+5;
//--- draw
   if(draw)
   {
      x1=x2-size;
      x2=x1+5;
      //--- draw line
      Line(x1,y,x1,m_y_max,m_color_text);
      //--- set font
      //FontAngleSet(0);
      //m_fontsize=-70;
      ////string oldFont=m_fontname;
      ////m_fontname="Consolas";
      //--- draw text
      RECT rct;
      rct.left=x2+5;
      for(uint i=0; i<=m_num_grid; i++,y-=m_dy_grid)
      {
         Line(x1,y,x2,y,m_color_text);
         if((int)i<m_scale_text_size)
         {
            rct.top=y;
            rct.bottom=y;
            DrawTextOut_A(&rct,m_scale_text[i],DT_SINGLELINE|DT_NOCLIP|DT_LEFT|DT_VCENTER);
         }
      }
      ////m_fontname=oldFont;
      //--- adjust data area
      m_data_area.right-=size;
   }
//--- return widht
   return(size);
}
void ChartCanvas::DrawGrid(void)
{
//--- check flag
   if(!IS_SHOW_GRID)
      return;
//--- variables
   int x1=m_data_area.left;
   int x2=m_data_area.right;
   int y =m_y_min;
//--- draw
   uint j=m_num_grid-((IS_SHOW_SCALE_TOP) ? 1 : 0);
   if(IS_SHOW_SCALE_BOTTOM)
   {
      y-=m_dy_grid;
      j--;
   }
   for(uint i=0; i<=j; i++,y-=m_dy_grid)
      LineHorizontalDott(x1,x2,y,m_color_grid);
}
void ChartCanvas::DrawDescriptors(void)
{
   /* RECT rct;
      rct.left=70;
      rct.top=0;
      DrawTextOut(&rct,m_descriptors[0],DT_SINGLELINE|DT_NOCLIP);*/
}
void ChartCanvas::DrawChart(void)
{
   for(uint i=0; i<m_data_total; i++)
      DrawData(i);
}
void ChartCanvas::VScaleParams(const double max,const double min,const uint grid,const bool redraw)
{
//--- check
   if(grid==0)
      return;
   if(max<=min)
      return;
   if(m_v_scale_max!=max || m_v_scale_min!=min || m_num_grid!=grid)
   {
      vScaleParamsChanged=true;
      m_v_scale_max=max;
      m_v_scale_min=min;
      m_num_grid=grid;
   }
//--- redraw
   if(m_data_total>0 && redraw)
      Redraw();
}
void ChartCanvas::ShowScaleTop(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_SCALE_TOP)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_SCALE_TOP;
      else
         m_show_flags&=~FLAG_SHOW_SCALE_TOP;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowScaleRight(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_SCALE_RIGHT)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_SCALE_RIGHT;
      else
         m_show_flags&=~FLAG_SHOW_SCALE_RIGHT;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowScaleLeft(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_SCALE_LEFT)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_SCALE_LEFT;
      else
         m_show_flags&=~FLAG_SHOW_SCALE_LEFT;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowScaleBottom(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_SCALE_BOTTOM)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_SCALE_BOTTOM;
      else
         m_show_flags&=~FLAG_SHOW_SCALE_BOTTOM;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowLegend(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_LEGEND)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_LEGEND;
      else
         m_show_flags&=~FLAG_SHOW_LEGEND;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowGrid(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_GRID)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_GRID;
      else
         m_show_flags&=~FLAG_SHOW_GRID;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ShowDescriptors(const bool flag,const bool redraw)
{
   if((m_allowed_show_flags&FLAG_SHOW_DESCRIPTORS)!=0)
   {
      if(flag)
         m_show_flags|=FLAG_SHOW_DESCRIPTORS;
      else
         m_show_flags&=~FLAG_SHOW_DESCRIPTORS;
      //--- redraw
      if(m_data_total>0 && redraw)
         Redraw();
   }
}
void ChartCanvas::ColorBackground(const uint value,const bool redraw)
{
   m_color_background=value;
//--- redraw
   if(m_data_total>0 && redraw)
      Redraw();
}
void ChartCanvas::ColorBorder(const uint value,const bool redraw)
{
   m_color_border=value;
//--- redraw
   if(m_data_total>0 && redraw)
      Redraw();
}
void ChartCanvas::ColorText(const uint value,const bool redraw)
{
   m_color_text=value;
   TextColorSet(m_color_text);
//--- redraw
   if(m_data_total>0 && redraw)
      Redraw();
}
void ChartCanvas::ShowFlags(const uint flags,const bool redraw)
{
   m_show_flags=flags&m_allowed_show_flags;
//--- redraw
   if(m_data_total>0 && redraw)
      Redraw();
}

