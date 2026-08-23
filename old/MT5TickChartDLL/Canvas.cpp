#include "StdAfx.h"
#include "Canvas.h"

WCHAR TIMETABLE[60][3]= {L"00",L"01",L"02",L"03",L"04",L"05",L"06",L"07",L"08",L"09",
                         L"10",L"11",L"12",L"13",L"14",L"15",L"16",L"17",L"18",L"19",
                         L"20",L"21",L"22",L"23",L"24",L"25",L"26",L"27",L"28",L"29",
                         L"30",L"31",L"32",L"33",L"34",L"35",L"36",L"37",L"38",L"39",
                         L"40",L"41",L"42",L"43",L"44",L"45",L"46",L"47",L"48",L"49",
                         L"50",L"51",L"52",L"53",L"54",L"55",L"56",L"57",L"58",L"59"
                        };

WCHAR WNUMBERS[10][2]= {L"0",L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9"};

char TIMETABLE_A[60][3]= {"00","01","02","03","04","05","06","07","08","09",
                          "10","11","12","13","14","15","16","17","18","19",
                          "20","21","22","23","24","25","26","27","28","29",
                          "30","31","32","33","34","35","36","37","38","39",
                          "40","41","42","43","44","45","46","47","48","49",
                          "50","51","52","53","54","55","56","57","58","59"
                         };

char WNUMBERS_A[10][2]= {"0","1","2","3","4","5","6","7","8","9"};


Canvas::Canvas(void): m_width(0),
   m_height(0),initialized(false),m_fontsize(-120)
{
   hbm=NULL;
   oldBitmap=NULL;
   hDCMem=NULL;
   hf_12=NULL;
   hf_09=NULL;
   hf_07=NULL;
   olfFont=NULL;
}

Canvas::~Canvas(void)
{
   if(hbm!=NULL)
      DeleteObject(hbm);
   if(hDCMem!=NULL)
      DeleteDC(hDCMem);
   if(hf_12!=NULL)
      DeleteObject(hf_12);
   if(hf_09!=NULL)
      DeleteObject(hf_09);
   if(hf_07!=NULL)
      DeleteObject(hf_07);
}
bool Canvas::IsInitialized()
{
   return(initialized);
}
bool Canvas::Create(HWND hWnd,const int width,const int height)
{
   if(!initialized)
   {
      m_width = width;
      m_height=height;

      /* long lfHeight;
         hDCMem = GetDC(NULL);
         lfHeight = -MulDiv(12, GetDeviceCaps(hDCMem, LOGPIXELSY), 72);
         ReleaseDC(NULL, hDCMem);*/

      canvasWindow=hWnd;
      HDC hDC = GetDC(canvasWindow);

      hf_12 = CreateFont(-12, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
      hf_09 = CreateFont(-9, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
      hf_07 = CreateFont(-7, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Consolas");

      ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
      bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmpInfo.bmiHeader.biWidth = m_width;
      bmpInfo.bmiHeader.biHeight = -m_height;
      bmpInfo.bmiHeader.biPlanes = 1;
      bmpInfo.bmiHeader.biBitCount = 32;
      bmpInfo.bmiHeader.biSizeImage = (uint)(m_width * m_height * 4);
      hDCMem = CreateCompatibleDC(hDC);
      olfFont = (HFONT)SelectObject(hDCMem, hf_12);
      hbm =CreateDIBSection(hDCMem, &bmpInfo, DIB_RGB_COLORS, (VOID**)&m_pixels, NULL, 0);
      oldBitmap=(HBITMAP)SelectObject(hDCMem, hbm);
      BitBlt(hDCMem, 0, 0, m_width, m_height, hDC, 0, 0, SRCCOPY);
      SetBkMode(hDCMem,TRANSPARENT);
      ReleaseDC(canvasWindow, hDC);

      initialized=true;
   }
   return(true);
}
void Canvas::CreatePixelFontSet(uint eraseColor)
{
   Erase(eraseColor);
   RECT rct;
   rct.left=0;
   rct.top=0;
   rct.bottom=12;

   char text[42] = "0123456789:.,aioeznrwstcykdpmujlbghfqvx;-";
   SelectFont(12);
   DrawTextOut_A(&rct,text,DT_SINGLELINE|DT_NOCLIP|DT_LEFT|DT_BOTTOM);

   for(int i=0; i<41; i++)
   {
      int xOffst = i*7;
      int nonZeroPixels=0;
      for(int k=0; k<11; k++)
      {
         for(int n=0; n<7; n++)
         {
            if(m_pixels[k*m_width+n+xOffst]!=eraseColor)
            {
               nonZeroPixels++;
               lettersData[i][0][nonZeroPixels-1]=k*m_width+n;
               lettersData[i][1][nonZeroPixels-1]=m_pixels[k*m_width+n+xOffst];
            }
         }
      }
      lettersBitIndsUsed[i]=nonZeroPixels;
   }
   Erase(eraseColor);
   rct.top=0;
   rct.bottom=8;

   //char text_2[42] = "0123456789:.,aioeznrwstcykdpmujlbghfqvx;-";
   SelectFont(9);
   DrawTextOut_A(&rct,text,DT_SINGLELINE|DT_NOCLIP|DT_LEFT|DT_BOTTOM);

   for(int i=0; i<41; i++)
   {
      int xOffst = i*5;
      int nonZeroPixels=0;
      for(int k=0; k<8; k++)
      {
         for(int n=0; n<5; n++)
         {
            if(m_pixels[k*m_width+n+xOffst]!=eraseColor)
            {
               nonZeroPixels++;
               lettersData_09[i][0][nonZeroPixels-1]=k*m_width+n;
               lettersData_09[i][1][nonZeroPixels-1]=m_pixels[k*m_width+n+xOffst];
            }
         }
      }
      lettersBitIndsUsed_09[i]=nonZeroPixels;
   }

   SelectFont(12);
   Erase(eraseColor);
}
bool Canvas::Destroy()
{
   if(hDCMem!=NULL)
   {
      DeleteObject(SelectObject(hDCMem,oldBitmap));
      hbm=NULL;
      SelectObject(hDCMem,olfFont);
      DeleteObject(hf_12);
      DeleteObject(hf_09);
      DeleteObject(hf_07);
      hf_12=NULL;
      hf_09=NULL;
      hf_07=NULL;

      DeleteDC(hDCMem);
      hDCMem=NULL;
      initialized=false;
   }
   return(true);
}
void Canvas::Update()
{
   if(hDCMem!=NULL)
   {
      HDC hDC = GetDC(canvasWindow);
      BitBlt(hDC, 0, 0, m_width, m_height, hDCMem, 0, 0, SRCCOPY);
      ReleaseDC(canvasWindow, hDC);
   }
}
void Canvas::Update(int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc)
{
   if(hDCMem!=NULL)
   {
      HDC hDC = GetDC(canvasWindow);
      BitBlt(hDC, nXDest, nYDest, nWidth, nHeight, hDCMem, nXSrc, nYSrc, SRCCOPY);
      ReleaseDC(canvasWindow, hDC);
   }
}
void Canvas::Erase(const uint clr)
{
   //ArrayFill(0,m_width*m_height,clr);
   memset(m_pixels,clr,m_width*m_height*4);
}
uint Canvas::PixelGet(const int x,const int y) const
{
   if((unsigned)x<(unsigned)m_width && (unsigned)y<(unsigned)m_height)
   {
      return(m_pixels[y*m_width+x]);
   }
   return(0);
}
void Canvas::PixelSet(const int x,const int y,const uint clr)
{
//--- check coordinates
   if((unsigned)x<(unsigned)m_width && (unsigned)y<(unsigned)m_height)
   {
      m_pixels[y*m_width+x]=clr;
   }
}
void Canvas::DrawBitTimeSepStamp_09(const char arrayOfSigns[],int arrSize, int x, int y, int leftLimit, int rightLimit)
{
   int i=0;
   int xOfst=0;

   for(int j=0; j<arrSize; j++)
   {
      if(arrayOfSigns[j]==NULL)
         break;

      xOfst=x+j*5;
      if(xOfst<leftLimit)
         continue;
      if(xOfst+16>rightLimit)
         break;

      i=y*m_width+xOfst;
      switch(arrayOfSigns[j])
      {
      case '0':
      {
         for(int k=0; k<lettersBitIndsUsed_09[0]; k++)
         {
            m_pixels[i+lettersData_09[0][0][k]]=lettersData_09[0][1][k];
         }
      }
      break;
      case '1':
      {
         for(int k=0; k<lettersBitIndsUsed_09[1]; k++)
         {
            m_pixels[i+lettersData_09[1][0][k]]=lettersData_09[1][1][k];
         }
      }
      break;
      case '2':
      {
         for(int k=0; k<lettersBitIndsUsed_09[2]; k++)
         {
            m_pixels[i+lettersData_09[2][0][k]]=lettersData_09[2][1][k];
         }
      }
      break;
      case '3':
      {
         for(int k=0; k<lettersBitIndsUsed_09[3]; k++)
         {
            m_pixels[i+lettersData_09[3][0][k]]=lettersData_09[3][1][k];
         }
      }
      break;
      case '4':
      {
         for(int k=0; k<lettersBitIndsUsed_09[4]; k++)
         {
            m_pixels[i+lettersData_09[4][0][k]]=lettersData_09[4][1][k];
         }
      }
      break;
      case '5':
      {
         for(int k=0; k<lettersBitIndsUsed_09[5]; k++)
         {
            m_pixels[i+lettersData_09[5][0][k]]=lettersData_09[5][1][k];
         }
      }
      break;
      case '6':
      {
         for(int k=0; k<lettersBitIndsUsed_09[6]; k++)
         {
            m_pixels[i+lettersData_09[6][0][k]]=lettersData_09[6][1][k];
         }
      }
      break;
      case '7':
      {
         for(int k=0; k<lettersBitIndsUsed_09[7]; k++)
         {
            m_pixels[i+lettersData_09[7][0][k]]=lettersData_09[7][1][k];
         }
      }
      break;
      case '8':
      {
         for(int k=0; k<lettersBitIndsUsed_09[8]; k++)
         {
            m_pixels[i+lettersData_09[8][0][k]]=lettersData_09[8][1][k];
         }
      }
      break;
      case '9':
      {
         for(int k=0; k<lettersBitIndsUsed_09[9]; k++)
         {
            m_pixels[i+lettersData_09[9][0][k]]=lettersData_09[9][1][k];
         }
      }
      break;
      case ':':
      {
         for(int k=0; k<lettersBitIndsUsed_09[10]; k++)
         {
            m_pixels[i+lettersData_09[10][0][k]]=lettersData_09[10][1][k];
         }
      }
      case '.':
      {
         for(int k=0; k<lettersBitIndsUsed_09[11]; k++)
         {
            m_pixels[i+lettersData_09[11][0][k]]=lettersData_09[11][1][k];
         }
      }
      break;
      case ',':
      {
         for(int k=0; k<lettersBitIndsUsed_09[12]; k++)
         {
            m_pixels[i+lettersData_09[12][0][k]]=lettersData_09[12][1][k];
         }
      }
      break;
      case 'a':
      {
         for(int k=0; k<lettersBitIndsUsed_09[13]; k++)
         {
            m_pixels[i+lettersData_09[13][0][k]]=lettersData_09[13][1][k];
         }
      }
      break;
      case 'i':
      {
         for(int k=0; k<lettersBitIndsUsed_09[14]; k++)
         {
            m_pixels[i+lettersData_09[14][0][k]]=lettersData_09[14][1][k];
         }
      }
      break;
      case 'o':
      {
         for(int k=0; k<lettersBitIndsUsed_09[15]; k++)
         {
            m_pixels[i+lettersData_09[15][0][k]]=lettersData_09[15][1][k];
         }
      }
      break;
      case 'e':
      {
         for(int k=0; k<lettersBitIndsUsed_09[16]; k++)
         {
            m_pixels[i+lettersData_09[16][0][k]]=lettersData_09[16][1][k];
         }
      }
      break;
      case 'z':
      {
         for(int k=0; k<lettersBitIndsUsed_09[17]; k++)
         {
            m_pixels[i+lettersData_09[17][0][k]]=lettersData_09[17][1][k];
         }
      }
      break;
      case 'n':
      {
         for(int k=0; k<lettersBitIndsUsed_09[18]; k++)
         {
            m_pixels[i+lettersData_09[18][0][k]]=lettersData_09[18][1][k];
         }
      }
      break;

      case 'r':
      {
         for(int k=0; k<lettersBitIndsUsed_09[19]; k++)
         {
            m_pixels[i+lettersData_09[19][0][k]]=lettersData_09[19][1][k];
         }
      }
      break;
      case 'w':
      {
         for(int k=0; k<lettersBitIndsUsed_09[20]; k++)
         {
            m_pixels[i+lettersData_09[20][0][k]]=lettersData_09[20][1][k];
         }
      }
      break;
      case 's':
      {
         for(int k=0; k<lettersBitIndsUsed_09[21]; k++)
         {
            m_pixels[i+lettersData_09[21][0][k]]=lettersData_09[21][1][k];
         }
      }
      break;
      case 't':
      {
         for(int k=0; k<lettersBitIndsUsed_09[22]; k++)
         {
            m_pixels[i+lettersData_09[22][0][k]]=lettersData_09[22][1][k];
         }
      }
      break;
      case 'c':
      {
         for(int k=0; k<lettersBitIndsUsed_09[23]; k++)
         {
            m_pixels[i+lettersData_09[23][0][k]]=lettersData_09[23][1][k];
         }
      }
      break;
      case 'y':
      {
         for(int k=0; k<lettersBitIndsUsed_09[24]; k++)
         {
            m_pixels[i+lettersData_09[24][0][k]]=lettersData_09[24][1][k];
         }
      }
      break;
      case 'k':
      {
         for(int k=0; k<lettersBitIndsUsed_09[25]; k++)
         {
            m_pixels[i+lettersData_09[25][0][k]]=lettersData_09[25][1][k];
         }
      }
      break;
      case 'd':
      {
         for(int k=0; k<lettersBitIndsUsed_09[26]; k++)
         {
            m_pixels[i+lettersData_09[26][0][k]]=lettersData_09[26][1][k];
         }
      }
      break;
      case 'p':
      {
         for(int k=0; k<lettersBitIndsUsed_09[27]; k++)
         {
            m_pixels[i+lettersData_09[27][0][k]]=lettersData_09[27][1][k];
         }
      }
      break;
      case 'm':
      {
         for(int k=0; k<lettersBitIndsUsed_09[28]; k++)
         {
            m_pixels[i+lettersData_09[28][0][k]]=lettersData_09[28][1][k];
         }
      }
      break;
      case 'u':
      {
         for(int k=0; k<lettersBitIndsUsed_09[29]; k++)
         {
            m_pixels[i+lettersData_09[29][0][k]]=lettersData_09[29][1][k];
         }
      }
      break;
      case 'j':
      {
         for(int k=0; k<lettersBitIndsUsed_09[30]; k++)
         {
            m_pixels[i+lettersData_09[30][0][k]]=lettersData_09[30][1][k];
         }
      }
      break;
      case 'l':
      {
         for(int k=0; k<lettersBitIndsUsed_09[31]; k++)
         {
            m_pixels[i+lettersData_09[31][0][k]]=lettersData_09[31][1][k];
         }
      }
      break;
      case 'b':
      {
         for(int k=0; k<lettersBitIndsUsed_09[32]; k++)
         {
            m_pixels[i+lettersData_09[32][0][k]]=lettersData_09[32][1][k];
         }
      }
      break;
      case 'g':
      {
         for(int k=0; k<lettersBitIndsUsed_09[33]; k++)
         {
            m_pixels[i+lettersData_09[33][0][k]]=lettersData_09[33][1][k];
         }
      }
      break;
      case 'h':
      {
         for(int k=0; k<lettersBitIndsUsed_09[34]; k++)
         {
            m_pixels[i+lettersData_09[34][0][k]]=lettersData_09[34][1][k];
         }
      }
      break;
      case 'f':
      {
         for(int k=0; k<lettersBitIndsUsed_09[35]; k++)
         {
            m_pixels[i+lettersData_09[35][0][k]]=lettersData_09[35][1][k];
         }
      }
      break;
      case 'q':
      {
         for(int k=0; k<lettersBitIndsUsed_09[36]; k++)
         {
            m_pixels[i+lettersData_09[36][0][k]]=lettersData_09[36][1][k];
         }
      }
      break;
      case 'v':
      {
         for(int k=0; k<lettersBitIndsUsed_09[37]; k++)
         {
            m_pixels[i+lettersData_09[37][0][k]]=lettersData_09[37][1][k];
         }
      }
      break;
      case 'x':
      {
         for(int k=0; k<lettersBitIndsUsed_09[38]; k++)
         {
            m_pixels[i+lettersData_09[38][0][k]]=lettersData_09[38][1][k];
         }
      }
      break;
      case ';':
      {
         for(int k=0; k<lettersBitIndsUsed_09[39]; k++)
         {
            m_pixels[i+lettersData_09[39][0][k]]=lettersData_09[39][1][k];
         }
      }
      break;
      case '-':
      {
         for(int k=0; k<lettersBitIndsUsed_09[40]; k++)
         {
            m_pixels[i+lettersData_09[40][0][k]]=lettersData_09[40][1][k];
         }
      }
      break;
      default:
         break;
      }
   }
}
void Canvas::DrawBitText_12(const char arrayOfSigns[],int arrSize, int x, int y, bool safeMode, int leftLimit, int rightLimit)
{
   int i=0;
   int xOfst=0;

   for(int j=0; j<arrSize; j++)
   {
      if(arrayOfSigns[j]==NULL)
         break;

      xOfst=x+j*7;
      if(safeMode)
      {
         if(xOfst<leftLimit)
            continue;
         if(xOfst+7>rightLimit)
            break;
      }

      i=y*m_width+xOfst;
      switch(arrayOfSigns[j])
      {
      case '0':
      {
         for(int k=0; k<lettersBitIndsUsed[0]; k++)
         {
            m_pixels[i+lettersData[0][0][k]]=lettersData[0][1][k];
         }
      }
      break;
      case '1':
      {
         for(int k=0; k<lettersBitIndsUsed[1]; k++)
         {
            m_pixels[i+lettersData[1][0][k]]=lettersData[1][1][k];
         }
      }
      break;
      case '2':
      {
         for(int k=0; k<lettersBitIndsUsed[2]; k++)
         {
            m_pixels[i+lettersData[2][0][k]]=lettersData[2][1][k];
         }
      }
      break;
      case '3':
      {
         for(int k=0; k<lettersBitIndsUsed[3]; k++)
         {
            m_pixels[i+lettersData[3][0][k]]=lettersData[3][1][k];
         }
      }
      break;
      case '4':
      {
         for(int k=0; k<lettersBitIndsUsed[4]; k++)
         {
            m_pixels[i+lettersData[4][0][k]]=lettersData[4][1][k];
         }
      }
      break;
      case '5':
      {
         for(int k=0; k<lettersBitIndsUsed[5]; k++)
         {
            m_pixels[i+lettersData[5][0][k]]=lettersData[5][1][k];
         }
      }
      break;
      case '6':
      {
         for(int k=0; k<lettersBitIndsUsed[6]; k++)
         {
            m_pixels[i+lettersData[6][0][k]]=lettersData[6][1][k];
         }
      }
      break;
      case '7':
      {
         for(int k=0; k<lettersBitIndsUsed[7]; k++)
         {
            m_pixels[i+lettersData[7][0][k]]=lettersData[7][1][k];
         }
      }
      break;
      case '8':
      {
         for(int k=0; k<lettersBitIndsUsed[8]; k++)
         {
            m_pixels[i+lettersData[8][0][k]]=lettersData[8][1][k];
         }
      }
      break;
      case '9':
      {
         for(int k=0; k<lettersBitIndsUsed[9]; k++)
         {
            m_pixels[i+lettersData[9][0][k]]=lettersData[9][1][k];
         }
      }
      break;
      case ':':
      {
         for(int k=0; k<lettersBitIndsUsed[10]; k++)
         {
            m_pixels[i+lettersData[10][0][k]]=lettersData[10][1][k];
         }
      }
      break;
      case '.':
      {
         for(int k=0; k<lettersBitIndsUsed[11]; k++)
         {
            m_pixels[i+lettersData[11][0][k]]=lettersData[11][1][k];
         }
      }
      break;
      case ',':
      {
         for(int k=0; k<lettersBitIndsUsed[12]; k++)
         {
            m_pixels[i+lettersData[12][0][k]]=lettersData[12][1][k];
         }
      }
      break;
      case 'a':
      {
         for(int k=0; k<lettersBitIndsUsed[13]; k++)
         {
            m_pixels[i+lettersData[13][0][k]]=lettersData[13][1][k];
         }
      }
      break;
      case 'i':
      {
         for(int k=0; k<lettersBitIndsUsed[14]; k++)
         {
            m_pixels[i+lettersData[14][0][k]]=lettersData[14][1][k];
         }
      }
      break;
      case 'o':
      {
         for(int k=0; k<lettersBitIndsUsed[15]; k++)
         {
            m_pixels[i+lettersData[15][0][k]]=lettersData[15][1][k];
         }
      }
      break;
      case 'e':
      {
         for(int k=0; k<lettersBitIndsUsed[16]; k++)
         {
            m_pixels[i+lettersData[16][0][k]]=lettersData[16][1][k];
         }
      }
      break;
      case 'z':
      {
         for(int k=0; k<lettersBitIndsUsed[17]; k++)
         {
            m_pixels[i+lettersData[17][0][k]]=lettersData[17][1][k];
         }
      }
      break;
      case 'n':
      {
         for(int k=0; k<lettersBitIndsUsed[18]; k++)
         {
            m_pixels[i+lettersData[18][0][k]]=lettersData[18][1][k];
         }
      }
      break;

      case 'r':
      {
         for(int k=0; k<lettersBitIndsUsed[19]; k++)
         {
            m_pixels[i+lettersData[19][0][k]]=lettersData[19][1][k];
         }
      }
      break;
      case 'w':
      {
         for(int k=0; k<lettersBitIndsUsed[20]; k++)
         {
            m_pixels[i+lettersData[20][0][k]]=lettersData[20][1][k];
         }
      }
      break;
      case 's':
      {
         for(int k=0; k<lettersBitIndsUsed[21]; k++)
         {
            m_pixels[i+lettersData[21][0][k]]=lettersData[21][1][k];
         }
      }
      break;
      case 't':
      {
         for(int k=0; k<lettersBitIndsUsed[22]; k++)
         {
            m_pixels[i+lettersData[22][0][k]]=lettersData[22][1][k];
         }
      }
      break;
      case 'c':
      {
         for(int k=0; k<lettersBitIndsUsed[23]; k++)
         {
            m_pixels[i+lettersData[23][0][k]]=lettersData[23][1][k];
         }
      }
      break;
      case 'y':
      {
         for(int k=0; k<lettersBitIndsUsed[24]; k++)
         {
            m_pixels[i+lettersData[24][0][k]]=lettersData[24][1][k];
         }
      }
      break;
      case 'k':
      {
         for(int k=0; k<lettersBitIndsUsed[25]; k++)
         {
            m_pixels[i+lettersData[25][0][k]]=lettersData[25][1][k];
         }
      }
      break;
      case 'd':
      {
         for(int k=0; k<lettersBitIndsUsed[26]; k++)
         {
            m_pixels[i+lettersData[26][0][k]]=lettersData[26][1][k];
         }
      }
      break;
      case 'p':
      {
         for(int k=0; k<lettersBitIndsUsed[27]; k++)
         {
            m_pixels[i+lettersData[27][0][k]]=lettersData[27][1][k];
         }
      }
      break;
      case 'm':
      {
         for(int k=0; k<lettersBitIndsUsed[28]; k++)
         {
            m_pixels[i+lettersData[28][0][k]]=lettersData[28][1][k];
         }
      }
      break;
      case 'u':
      {
         for(int k=0; k<lettersBitIndsUsed[29]; k++)
         {
            m_pixels[i+lettersData[29][0][k]]=lettersData[29][1][k];
         }
      }
      break;
      case 'j':
      {
         for(int k=0; k<lettersBitIndsUsed[30]; k++)
         {
            m_pixels[i+lettersData[30][0][k]]=lettersData[30][1][k];
         }
      }
      break;
      case 'l':
      {
         for(int k=0; k<lettersBitIndsUsed[31]; k++)
         {
            m_pixels[i+lettersData[31][0][k]]=lettersData[31][1][k];
         }
      }
      break;
      case 'b':
      {
         for(int k=0; k<lettersBitIndsUsed[32]; k++)
         {
            m_pixels[i+lettersData[32][0][k]]=lettersData[32][1][k];
         }
      }
      break;
      case 'g':
      {
         for(int k=0; k<lettersBitIndsUsed[33]; k++)
         {
            m_pixels[i+lettersData[33][0][k]]=lettersData[33][1][k];
         }
      }
      break;
      case 'h':
      {
         for(int k=0; k<lettersBitIndsUsed[34]; k++)
         {
            m_pixels[i+lettersData[34][0][k]]=lettersData[34][1][k];
         }
      }
      break;
      case 'f':
      {
         for(int k=0; k<lettersBitIndsUsed[35]; k++)
         {
            m_pixels[i+lettersData[35][0][k]]=lettersData[35][1][k];
         }
      }
      break;
      case 'q':
      {
         for(int k=0; k<lettersBitIndsUsed[36]; k++)
         {
            m_pixels[i+lettersData[36][0][k]]=lettersData[36][1][k];
         }
      }
      break;
      case 'v':
      {
         for(int k=0; k<lettersBitIndsUsed[37]; k++)
         {
            m_pixels[i+lettersData[37][0][k]]=lettersData[37][1][k];
         }
      }
      break;
      case 'x':
      {
         for(int k=0; k<lettersBitIndsUsed[38]; k++)
         {
            m_pixels[i+lettersData[38][0][k]]=lettersData[38][1][k];
         }
      }
      break;
      case ';':
      {
         for(int k=0; k<lettersBitIndsUsed[39]; k++)
         {
            m_pixels[i+lettersData[39][0][k]]=lettersData[39][1][k];
         }
      }
      break;
      case '-':
      {
         for(int k=0; k<lettersBitIndsUsed[40]; k++)
         {
            m_pixels[i+lettersData[40][0][k]]=lettersData[40][1][k];
         }
      }
      break;
      default:
         break;
      }
   }
}
void Canvas::LineVertical(int x,int y1,int y2,const uint clr)
{
   //int tmp;
//--- sort by Y
   if(y1>y2)
   {
      //tmp=y1;
      //Swap without temporary variable
      y1 +=y2;
      y2 =y1-y2;
      y1 -=y2;
   }
//--- stay withing image boundaries
   if(y2>=0 && y1<m_height && (unsigned)x<(unsigned)m_width)
   {
      if(y1<0)
      {
         y1=0;
      }
      if(y2>=m_height)
      {
         y2=m_height-1;
      }
//--- draw line
      int index=y1*m_width+x;
      for(int i=y1; i<=y2; i++,index+=m_width)
      {
         m_pixels[index]=clr;
      }
   }
}
//y1 and y2 sorted. All points stay withing image boundaries
void Canvas::SafeSortedLineVertical(int x,int y1,int y2,const uint clr)
{
   int index=y1*m_width+x;
   for(int i=y1; i<=y2; i++,index+=m_width)
   {
      m_pixels[index]=clr;
   }
}
void Canvas::LineVerticalDott(int x,int y1,int y2,const uint clr)
{
   //int tmp;
//--- sort by Y
   if(y1>y2)
   {
      //tmp=y1;
      //Swap without temporary variable
      y1 +=y2;
      y2 =y1-y2;
      y1 -=y2;
   }
//--- stay withing image boundaries
   if(y2>=0 && y1<m_height && (unsigned)x<(unsigned)m_width)
   {
      if(y1<0)
      {
         y1=0;
      }
      if(y2>=m_height)
      {
         y2=m_height-1;
      }
//--- draw line
      int index=y1*m_width+x;
      y2-=2;
      for(int i=y1; i<=y2; i+=7,index+=7*m_width)
      {
         m_pixels[index]=clr;
         m_pixels[index+m_width]=clr;
         m_pixels[index+2*m_width]=clr;
      }
   }
}
void Canvas::LineHorizontal(int x1,int x2,int y,const uint clr)
{
   //int tmp;
//--- sort by X
   if(x1>x2)
   {
      //tmp=x1;
      //Swap without temporary variable
      x1 +=x2;
      x2 =x1-x2;
      x1 -=x2;
   }
//--- stay withing image boundaries
   if(x2>=0 && x1<m_width && (unsigned)y<(unsigned)m_height)
   {
      if(x1<0)
      {
         x1=0;
      }
      if(x2>=m_width)
      {
         x2=m_width-1;
      }
//--- draw line
      ArrayFill(y*m_width+x1,(x2-x1)+1,clr);
   }
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void Canvas::SafeSortedLineHorizontal(int x1,int x2,int y,const uint clr)
{
   //x1<x2, all points withing image boundaries
   ArrayFill(y*m_width+x1,(x2-x1)+1,clr);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void Canvas::LineHorizontalDott(int x1,int x2,int y,const uint clr)
{
   //int tmp;
//--- sort by X
   if(x1>x2)
   {
      //tmp=x1;
      //Swap without temporary variable
      x1 +=x2;
      x2 =x1-x2;
      x1 -=x2;
   }
//--- stay withing image boundaries
   if(x2>=0 && x1<m_width && (unsigned)y<(unsigned)m_height)
   {
      if(x1<0)
      {
         x1=0;
      }
      if(x2>=m_width)
      {
         x2=m_width-1;
      }
      int index=y*m_width+x1;
      x2-=2;
      for(int i=x1; i<=x2; i+=7,index+=7)
      {
         m_pixels[index]=clr;
         m_pixels[index+1]=clr;
         m_pixels[index+2]=clr;
      }
   }
}
void Canvas::SafeSortedLineHorizontalDott(int x1,int x2,int y,const uint clr)
{
   int index=y*m_width+x1;
   x2-=2;
   for(int i=x1; i<=x2; i+=7,index+=7)
   {
      m_pixels[index]=clr;
      m_pixels[index+1]=clr;
      m_pixels[index+2]=clr;
   }
}
void Canvas::Line(int x1,int y1,int x2,int y2,const uint clr)
{
//--- stay withing image boundaries
   if((unsigned)x1<(unsigned)m_width && (unsigned)x2<(unsigned)m_width
         && (unsigned)y1<(unsigned)m_height && (unsigned)y2<(unsigned)m_height)
   {
      //--- get length by X and Y
      //With optimization flag on, ompiler replace 'abs' with efficient code
      int dx=abs(x2-x1);
      int dy=abs(y2-y1);
      if(dx==0)
      {
         //--- vertical line
         LineVertical(x1,y1,y2,clr);
         return;
      }
      if(dy==0)
      {
         //--- horizontal line
         LineHorizontal(x1,x2,y1,clr);
         return;
      }
//--- get direction by X and Y
      int  sx=(x1<x2)? 1 : -1;
      int  sy=(y1<y2)? 1 : -1;
      int  er=dx-dy;
      bool draw=false;
//--- continue to draw line
      while(x1!=x2 || y1!=y2)
      {
         if((unsigned)x1<(unsigned)m_width &&
               (unsigned)y1<(unsigned)m_height)
         {
            //--- draw pixel
            m_pixels[y1*m_width+x1]=clr;
            draw=true;
         }
         else
         {
            if(draw)
            {
               return;
            }
         }
         //--- get coordinates of next pixel
         int er2=er<<1;
         if(er2>-dy)
         {
            er-=dy;
            x1+=sx;
         }
         if(er2<dx)
         {
            er+=dx;
            y1+=sy;
         }
      }
//--- set pixel at the end
      //PixelSet(x2,y2,clr);
      m_pixels[y2*m_width+x2]=clr;
   }
}
void Canvas::Rectangle(int x1,int y1,int x2,int y2,const uint clr)
{
   LineHorizontal(x1,x2,y1,clr);
   LineVertical(x2,y1,y2,clr);
   LineHorizontal(x2,x1,y2,clr);
   LineVertical(x1,y2,y1,clr);
}
void Canvas::SafeSortedRectangle(int x1,int y1,int x2,int y2,const uint clr)
{
   SafeSortedLineHorizontal(x1,x2,y1,clr);
   SafeSortedLineVertical(x2,y1,y2,clr);
   SafeSortedLineHorizontal(x1,x2,y2,clr);
   SafeSortedLineVertical(x1,y1,y2,clr);
}
void Canvas::Triangle(int x1,int y1,int x2,int y2,int x3,int y3,const uint clr)
{
   Line(x1,y1,x2,y2,clr);
   Line(x2,y2,x3,y3,clr);
   Line(x3,y3,x1,y1,clr);
}
//+------------------------------------------------------------------+
//| Draw circle according to Bresenham's algorithm                   |
//+------------------------------------------------------------------+
void Canvas::Circle(int x,int y,int r,const uint clr)
{
   int f   =1-r;
   int dd_x=1;
   int dd_y=-2*r;
   int dx  =0;
   int dy  =r;
   int xx,yy;
//--- draw
   while(dy>=dx)
   {
      xx=x+dx;
      if(xx>=0 && xx<m_width)
      {
         yy=y+dy;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
         yy=y-dy;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
      }
      xx=x-dx;
      if(xx>=0 && xx<m_width)
      {
         yy=y+dy;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
         yy=y-dy;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
      }
      xx=x+dy;
      if(xx>=0 && xx<m_width)
      {
         yy=y+dx;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
         yy=y-dx;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
      }
      xx=x-dy;
      if(xx>=0 && xx<m_width)
      {
         yy=y+dx;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
         yy=y-dx;
         if(yy>=0 && yy<m_height)
         {
            m_pixels[yy*m_width+xx]=clr;
         }
      }
      //---
      if(f>=0)
      {
         dy--;
         dd_y+=2;
         f+=dd_y;
      }
      dx++;
      dd_x+=2;
      f+=dd_x;
   }
}
//+------------------------------------------------------------------+
//| Draw ellipse according to Bresenham's algorithm                  |
//+------------------------------------------------------------------+
void Canvas::Ellipse(int x1,int y1,int x2,int y2,const uint clr)
{
   int x,y;
   int rx,ry;
   int dx,dy;
   int xx,yy;
   int rx_sq,ry_sq;
   int f;
   int tmp;
//--- handle extreme conditions
   if(x1==x2)
   {
      if(y1==y2)
      {
         PixelSet(x1,y1,clr);
      }
      else
      {
         LineVertical(x1,y1,y2,clr);
      }
      return;
   }
   if(y1==y2)
   {
      LineHorizontal(x1,x2,y1,clr);
      return;
   }
//--- sort by X
   if(x1>x2)
   {
      tmp=x1;
      x1 =x2;
      x2 =tmp;
   }
//--- sort by Y
   if(y1>y2)
   {
      tmp=y1;
      y1 =y2;
      y2 =tmp;
   }
   x =(x2+x1)>>1;
   y =(y2+y1)>>1;
   rx=(x2-x1)>>1;
   ry=(y2-y1)>>1;
   dx=0;
   dy=ry;
   rx_sq=rx*rx;
   ry_sq=ry*ry;
   f=(rx_sq<<1)*((dy-1)*dy)+rx_sq+(ry_sq<<1)*(1-rx_sq);
   while(rx_sq*dy>ry_sq*dx)
   {
      yy=y+dy;
      if(yy>=0 && yy<m_height)
      {
         yy*=m_width;
         xx=x+dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
         xx=x-dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
      }
      yy=y-dy;
      if(yy>=0 && yy<m_height)
      {
         yy*=m_width;
         xx=x+dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
         xx=x-dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
      }
      if(f>=0)
      {
         dy--;
         f-=(rx_sq<<2)*dy;
      }
      f+=(ry_sq<<1)*(3+(dx<<1));
      dx++;
   }
   f=(ry_sq<<1)*(dx+1)*dx+(rx_sq<<1)*(dy*(dy-2)+1)+(1-(rx_sq<<1))*ry_sq;
   while(dy>=0)
   {
      yy=y+dy;
      if(yy>=0 && yy<m_height)
      {
         yy*=m_width;
         xx=x+dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
         xx=x-dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
      }
      yy=y-dy;
      if(yy>=0 && yy<m_height)
      {
         yy*=m_width;
         xx=x+dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
         xx=x-dx;
         if(xx>=0 && xx<m_width)
         {
            m_pixels[yy+xx]=clr;
         }
      }
      if(f<=0)
      {
         dx++;
         f+=(ry_sq<<2)*dx;
      }
      dy--;
      f+=(rx_sq<<1)*(3-(dy<<1));
   }
}
void Canvas::FillRectangle(int x1,int y1,int x2,int y2,const uint clr)
{
   int tmp;
//--- sort vertexes
   if(x2<x1)
   {
      tmp=x1;
      x1 =x2;
      x2 =tmp;
   }
   if(y2<y1)
   {
      tmp=y1;
      y1 =y2;
      y2 =tmp;
   }
//--- stay withing screen boundaries
   if(x2>=0 && y2>=0 && x1<m_width && y1<m_height)
   {
      if(x1<0)
      {
         x1=0;
      }
      if(y1<0)
      {
         y1=0;
      }
      if(x2>=m_width)
      {
         x2=m_width -1;
      }
      if(y2>=m_height)
      {
         y2=m_height-1;
      }
      int len=(x2-x1)+1;
//--- set pixels
      for(; y1<=y2; y1++)
      {
         ArrayFill(y1*m_width+x1,len,clr);
      }
   }
}
void Canvas::SafeSortedFillRectangle(int x1,int y1,int x2,int y2,const uint clr)
{
   int len=(x2-x1)+1;
//--- set pixels
   for(; y1<=y2; y1++)
   {
      ArrayFill(y1*m_width+x1,len,clr);
   }
}
//+------------------------------------------------------------------+
//| Draw filled triangle                                             |
//+------------------------------------------------------------------+
void Canvas::FillTriangle(int x1,int y1,int x2,int y2,int x3,int y3,const uint clr)
{
   int    xx1,xx2,tmp;
   double k1=0,k2=0,xd1,xd2;
//--- sort vertexes from lesser to greater
   if(y1>y2)
   {
      tmp=y2;
      y2 =y1;
      y1 =tmp;
      tmp=x2;
      x2 =x1;
      x1=tmp;
   }
   if(y1>y3)
   {
      tmp=y1;
      y1 =y3;
      y3 =tmp;
      tmp=x1;
      x1 =x3;
      x3 =tmp;
   }
   if(y2>y3)
   {
      tmp=y2;
      y2 =y3;
      y3 =tmp;
      tmp=x2;
      x2 =x3;
      x3 =tmp;
   }
//--- all vertexes are out of image boundaries
   if(y3<0 || y1>m_height)
   {
      return;
   }
   if(x1<0 && x2<0 && x3<0)
   {
      return;
   }
   if(x1>m_width && x2>m_width && x3>m_width)
   {
      return;
   }
//--- find coefficients of lines
   if((tmp=y1-y2)!=0)
   {
      k1=(x1-x2)/(double)tmp;
   }
   if((tmp=y1-y3)!=0)
   {
      k2=(x1-x3)/(double)tmp;
   }
//---
   xd1=x1;
   xd2=x1;
//---
   for(int i=y1; i<=y3; i++)
   {
      if(i==y2)
      {
         if((tmp=y2-y3)!=0)
         {
            k1=(x2-x3)/(double)tmp;
         }
         xd1=x2;
      }
      //--- calculate new boundaries of triangle line
      xx1 =(int)xd1;
      xd1+=k1;
      xx2 =(int)xd2;
      xd2+=k2;
      //--- triangle line is out of screen boundaries
      if(i<0 || i>=m_height)
      {
         continue;
      }
      //--- sort
      if(xx1>xx2)
      {
         tmp=xx1;
         xx1=xx2;
         xx2=tmp;
      }
      //--- line is out of screen boundaries
      if(xx2<0 || xx1>=m_width)
      {
         continue;
      }
      //--- draw only what is within screen boundaries
      if(xx1<0)
      {
         xx1=0;
      }
      if(xx2>=m_width)
      {
         xx2=m_width-1;
      }
      //--- draw horizontal line of triangle
      ArrayFill(i*m_width+xx1,xx2-xx1,clr);
   }
}
void Canvas::FillCircle(int x,int y,int r,const uint clr)
{
   int f   =1-r;
   int dd_x=1;
   int dd_y=-2*r;
   int dx  =0;
   int dy  =r;
//--- draw
   while(dy>=dx)
   {
      LineHorizontal(x-dy,x+dy,y-dx,clr);
      LineHorizontal(x-dy,x+dy,y+dx,clr);
      //---
      if(f>=0)
      {
         LineHorizontal(x-dx,x+dx,y-dy,clr);
         LineHorizontal(x-dx,x+dx,y+dy,clr);
         dy--;
         dd_y+=2;
         f+=dd_y;
      }
      dx++;
      dd_x+=2;
      f+=dd_x;
   }
}
//+------------------------------------------------------------------+
//| Draw filled ellipse                                              |
//+------------------------------------------------------------------+
void Canvas::FillEllipse(int x1,int y1,int x2,int y2,const uint clr)
{
   int x,y;
   int rx,ry;
   int dx,dy;
   int rx_sq,ry_sq;
   int f;
   int tmp;
//--- handle extreme conditions
   if(x1==x2)
   {
      if(y1==y2)
      {
         PixelSet(x1,y1,clr);
      }
      else
      {
         LineVertical(x1,y1,y2,clr);
      }
      return;
   }
   if(y1==y2)
   {
      LineHorizontal(x1,x2,y1,clr);
      return;
   }
//--- sort by X
   if(x1>x2)
   {
      tmp=x1;
      x1 =x2;
      x2 =tmp;
   }
//--- sort by Y
   if(y1>y2)
   {
      tmp=y1;
      y1 =y2;
      y2 =tmp;
   }
   x =(x2+x1)>>1;
   y =(y2+y1)>>1;
   rx=(x2-x1)>>1;
   ry=(y2-y1)>>1;
   dx=0;
   dy=ry;
   rx_sq=rx*rx;
   ry_sq=ry*ry;
   f=(rx_sq<<1)*((dy-1)*dy)+rx_sq+(ry_sq<<1)*(1-rx_sq);
   while(rx_sq*dy>ry_sq*(dx))
   {
      LineHorizontal(x-dx,x+dx,y+dy,clr);
      LineHorizontal(x-dx,x+dx,y-dy,clr);
      if(f>=0)
      {
         dy--;
         f-=(rx_sq<<2)*dy;
      }
      f+=(ry_sq<<1)*(3+(dx<<1));
      dx++;
   }
   f=(ry_sq<<1)*(dx+1)*dx+(rx_sq<<1)*(dy*(dy-2)+1)+(1-(rx_sq<<1))*ry_sq;
   while(dy>=0)
   {
      LineHorizontal(x-dx,x+dx,y+dy,clr);
      LineHorizontal(x-dx,x+dx,y-dy,clr);
      if(f<=0)
      {
         dx++;
         f+=(ry_sq<<2)*dx;
      }
      dy--;
      f+=(rx_sq<<1)*(3-(dy<<1));
   }
}
//void Canvas::DrawTextOut(LPRECT lprc,LPCTSTR text,const uint clr,uint alignment)
//{
//    SetTextColor(hDCMem, clr);
//    //alignment=DT_SINGLELINE | DT_NOCLIP//DrawText is somewhat faster when DT_NOCLIP is used.
//    //DT_BOTTOM,DT_CENTER,DT_LEFT,DT_RIGHT,DT_TOP,DT_VCENTER
//    DrawTextW(hDCMem, text,-1,lprc,alignment);
//}
void Canvas::DrawTextOut(LPRECT lprc,LPCTSTR text,uint alignment)
{
   //alignment=DT_SINGLELINE | DT_NOCLIP//DrawText is somewhat faster when DT_NOCLIP is used.
   //DT_BOTTOM,DT_CENTER,DT_LEFT,DT_RIGHT,DT_TOP,DT_VCENTER
   DrawTextW(hDCMem, text,-1,lprc,alignment);
}
void Canvas::DrawTextOut_A(LPRECT lprc,LPCSTR text,uint alignment)
{
   //alignment=DT_SINGLELINE | DT_NOCLIP//DrawText is somewhat faster when DT_NOCLIP is used.
   //DT_BOTTOM,DT_CENTER,DT_LEFT,DT_RIGHT,DT_TOP,DT_VCENTER
   DrawTextA(hDCMem, text,-1,lprc,alignment);
}
void Canvas::TextColorSet(const uint clr)
{
   SetTextColor(hDCMem, clr);
}

void Canvas::ArrayFill(int start, int count, int val)
{
   //memset(&m_pixels[start],val,count*4);
   unsigned long long fillValue = val;
   fillValue = fillValue << 32 |val;
   unsigned long long* dest = (unsigned long long*)(&m_pixels[start]);
   int length = count;
   while(length >= 2)
   {
      *dest = fillValue;
      dest++;
      length -= 2;
   }
   //In case 'count' is odd
   m_pixels[start+count-1]=val;
}
bool Canvas::SelectFont(int size)
{
   switch(size)
   {
   case 7:
      SelectObject(hDCMem,hf_07);
      m_fontsize=-70;
      break;
   case 9:
      SelectObject(hDCMem,hf_09);
      m_fontsize=-90;
      break;
   case 12:
      SelectObject(hDCMem,hf_12);
      m_fontsize=-120;
      break;
   default:
      SelectObject(hDCMem,hf_12);
      m_fontsize=-120;
   }
   return(true);
}
//bool Canvas::FontSet(LPCTSTR name,const int size,const uint angle, const uint weight, bool italic, bool underline, bool strikeout)
//{
//   hf = CreateFont(size, 0, 0, angle, weight, italic, underline, strikeout, 0, 0, 0, 0, 0, name);
//   DeleteObject(SelectObject(hDCMem,hf));
//   //return(TextSetFont(m_fontname,m_fontsize,m_fontflags,m_fontangle));
//   return(true);
//}
int Canvas::TextWidth(LPCTSTR text)
{
   SIZE  psizl;
   GetTextExtentPoint32W(hDCMem,text,(int)_tcslen(text),&psizl);
//--- result
   return(psizl.cx);
}
int Canvas::TextWidth_A(LPCSTR text)
{
   SIZE  psizl;
   GetTextExtentPoint32A(hDCMem,text,(int)strlen(text),&psizl);
//--- result
   return(psizl.cx);
}
int Canvas::TextHeight(LPCTSTR text)
{
   SIZE  psizl;
   GetTextExtentPoint32W(hDCMem,text,(int)_tcslen(text),&psizl);
//--- result
   return(psizl.cy);
}
void Canvas::TextSize(LPCTSTR text,int &width,int &height)
{
   SIZE  psizl;
   GetTextExtentPoint32W(hDCMem,text,(int)_tcslen(text),&psizl);
   width=psizl.cx;
   height=psizl.cy;
}
