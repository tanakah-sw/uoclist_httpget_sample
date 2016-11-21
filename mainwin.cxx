// mainwin.cxx

#include "mainwin.h"
#include "resource.h"

#include "tips.h"

void mainWin::alloc_bufGDI(int width, int scanline, int bitcount)
{
  static int l_bitCount=-1;
  static int l_lineWidth=-1;
  static int l_bytesPerLine=-1;
  static int l_scanLine=-1;

  int bytesPerLine=(width*((bitcount+7)/8*8)+31)/32*4; // 4bytes=32bits align
  if((bitcount==l_bitCount)&&(bytesPerLine==l_bytesPerLine)&&
     (l_lineWidth==width)&&(l_scanLine==scanline)){
    return;
  }
  l_bitCount=bitcount;
  l_bytesPerLine=bytesPerLine;
  l_lineWidth=width;
  l_scanLine=scanline;

  if(l_bitCount==-1)     return;
  if(l_bytesPerLine==-1) return;
  if(l_lineWidth==-1)    return;
  if(l_scanLine==-1)     return;

  BITMAPINFOHEADER bi;
  bi.biSize=sizeof(BITMAPINFOHEADER);
  bi.biWidth=l_lineWidth;
  bi.biHeight=-l_scanLine; // reverse
  bi.biPlanes=1;
  bi.biBitCount=(WORD)((l_bitCount+7)/8*8);
  bi.biCompression=((l_bitCount>8)&&(l_bitCount<16))? BI_BITFIELDS: BI_RGB;
  bi.biSizeImage=l_bytesPerLine*l_scanLine;
  bi.biXPelsPerMeter=bi.biYPelsPerMeter=0;
  bi.biClrUsed=bi.biClrImportant=0;
  memcpy(&(pbi->bmiHeader), &bi, sizeof(BITMAPINFOHEADER));

  switch(l_bitCount){
  case 8:
    {
      for(int c=0; c<256; c++){
        pbi->bmiColors[c].rgbBlue=(c&0xff);
        pbi->bmiColors[c].rgbGreen=(c&0xff);
        pbi->bmiColors[c].rgbRed=(c&0xff);
      }
    }
    break;
  case 10:
    {
      for(int c=0; c<3; c++){
        pbi->bmiColors[c].rgbBlue=0xff; // 0xfc;
        pbi->bmiColors[c].rgbGreen=0x03;
        pbi->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  case 12:
    {
      for(int c=0; c<3; c++){
        pbi->bmiColors[c].rgbBlue=0xff; // 0xf0;
        pbi->bmiColors[c].rgbGreen=0x0f;
        pbi->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  case 14:
    {
      for(int c=0; c<3; c++){
        pbi->bmiColors[c].rgbBlue=0xff; // 0xc0;
        pbi->bmiColors[c].rgbGreen=0x3f;
        pbi->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  }
  if(hbmp!=NULL) DeleteObject(hbmp);
  hbmp=CreateDIBSection(NULL, pbi, DIB_RGB_COLORS, (void **)(&m_buffer), NULL, 0);
  memset(m_buffer, 0, bi.biSizeImage);
}

void mainWin::nonPassiveRefreshImage()
{
  RECT rc;
  GetClientRect(m_hwnd, &rc);
  rc.left=400;
  InvalidateRect(m_hdlg, &rc, FALSE);
}

void mainWin::registerClass()
{
  WNDCLASSEX wc_main;

  memset(&wc_main, 0, sizeof(WNDCLASSEX));
  wc_main.cbSize=sizeof(WNDCLASSEX);
  wc_main.style=CS_HREDRAW|CS_VREDRAW;
  wc_main.lpfnWndProc=(WNDPROC)(mainWin::wndProcMain);
  wc_main.cbClsExtra=wc_main.cbWndExtra=0;
  wc_main.hInstance=m_hAppInstance;
  wc_main.hIcon=LoadIcon(m_hAppInstance, IDI_APPLICATION);
  wc_main.hIconSm=LoadIcon(m_hAppInstance, IDI_APPLICATION);
  wc_main.hCursor=LoadCursor(NULL, IDC_ARROW);

  wc_main.hbrBackground=(HBRUSH)(COLOR_3DFACE+1);
  //  wc_main.hbrBackground=(HBRUSH)(NULL_BRUSH);
  //  wc_main.hbrBackground=(HBRUSH)(BLACK_BRUSH);
  //  wc_main.hbrBackground=(HBRUSH)(WHITE_BRUSH);

  wc_main.lpszMenuName=MAKEINTRESOURCE(IDR_MENU1);
  //  wc_main.lpszMenuName=NULL;
  wc_main.lpszClassName=TEXT(MAINWIN_CLASS_NAME);
  if(!RegisterClassEx(&wc_main)) PostQuitMessage(0);

}  

void mainWin::unregisterClass()
{
  UnregisterClass(TEXT(MAINWIN_CLASS_NAME), m_hAppInstance);
}

mainWin::mainWin(HINSTANCE hinstance)
{
  m_hAppInstance=hinstance;

  roi_w=roi_h=roi_b=0;
  hbmp=NULL; m_buffer=NULL;
  pbi=(PBITMAPINFO)malloc(SIZEOFBITMAPINFO); memset(pbi, 0, SIZEOFBITMAPINFO);

  sprintf(iniFileName, "%s\\settings.ini", homeFolderName);
  //  loadSettingIni(iniFileName, &options);

  registerClass();
  m_hwnd=CreateWindow(TEXT(MAINWIN_CLASS_NAME), TEXT("MyTestSample"),
                      WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|
		      WS_VISIBLE|WS_SYSMENU|WS_CAPTION|
		      WS_MINIMIZEBOX,
                      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                      NULL, NULL, m_hAppInstance, (LPVOID)this);
    
  m_hdlg=CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG1), m_hwnd,
                           (DLGPROC)dlgProcMain, (LONG_PTR)this);

  // m_hdlg adjust
  RECT crc;
  GetClientRect(m_hdlg, &crc);
  MoveWindow(m_hdlg, 0, 0, crc.right-crc.left, crc.bottom-crc.top, TRUE);

  // m_hwnd adjust
  RECT prc;
  MyGetWindowRect(m_hwnd, &prc);
  int xp=prc.left, yp=prc.top;
  SetRect(&prc, 0, 0, crc.right-crc.left, crc.bottom-crc.top);
  DWORD style=GetWindowLongPtr(m_hwnd, GWL_STYLE);
  AdjustWindowRect(&prc, style, FALSE); // AdjustWindowRect(&prc, style, GetMenu(m_hwnd));
  SetWindowPos(m_hwnd, HWND_NOTOPMOST, xp, yp, prc.right-prc.left, prc.bottom-prc.top,
	       SWP_NOZORDER| SWP_NOACTIVATE);

  ShowWindow(m_hdlg, SW_SHOWNA);
  ShowWindow(m_hwnd, SW_SHOWNORMAL);
  UpdateWindow(m_hwnd);
}

mainWin::~mainWin()
{
  if(hbmp!=NULL) DeleteObject(hbmp);
  if(pbi!=NULL) free(pbi);

  //  saveSettingIni(iniFileName, &options);
  unregisterClass();
}

LRESULT CALLBACK mainWin::wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  mainWin *_this=(mainWin *)GetWindowLong(hwnd, GWL_USERDATA);
  switch(msg){
  case WM_COMMAND:
    {
      switch(LOWORD(wparam)){
      case IDM_QUIT:
        DestroyWindow(hwnd);
        break;
      }
    }
    break;

  case WM_CREATE:
    _this=(mainWin *)(((CREATESTRUCT *)lparam)->lpCreateParams);
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)_this);
    break;

  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

LRESULT CALLBACK mainWin::dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  mainWin *_this=(mainWin *)GetWindowLongPtr(hdlg, GWLP_USERDATA);
  switch(msg){

  case WM_COMMAND:
    {
      switch(HIWORD(wparam)){
      case CBN_SELCHANGE:
        {
        }
        break;
      }

      switch(LOWORD(wparam)){
      case IDC_BUTTON_PROC:
        {
          //          HMODULE hmod_richedit=LoadLibrary("RichEd32.dll");
          _this->geteventcal.Init(_this->m_hAppInstance);
          char url[256];
          strcpy(url, "http://www.google.co.jp");
          _this->geteventcal.DispFetchedData(url, hdlg);
          return TRUE;
        }
        break;
      case IDC_BUTTON_QUIT:
        {
          DestroyWindow(_this->m_hwnd);
          return TRUE;
        }
        break;
      }
    }
    break;
  case WM_PAINT:
    {
      int sx=0, sy=0, sw=_this->roi_w, sh=_this->roi_h;
      int dx=400, dy=10, dw=M_WIDTH, dh=M_HEIGHT;

      HDC hdc=GetDC(hdlg);
      SetBkMode(hdc, TRANSPARENT);
      SetStretchBltMode(hdc, COLORONCOLOR); // SetStretchBltMode(hdc, HALFTONE);

      /*
      // DrawDIB
      {
        StretchDIBits(
                      hdc,
                      dx, dy, dw, dh,
                      sx, sy, sw, sh,
                      _this->bufAPX[_this->curIndex], _this->pbi, DIB_RGB_COLORS, SRCCOPY);
      }
      */

      // DrawDDB
      {
        HDC hmemdc=CreateCompatibleDC(hdc);
        HBITMAP holdbmp=(HBITMAP)SelectObject(hmemdc, _this->hbmp);
        StretchBlt(
                   hdc, dx, dy, dw, dh,
                   hmemdc, sx, sy, sw, sh,
                   SRCCOPY);
        SelectObject(hmemdc, holdbmp);
        DeleteObject(hmemdc);
      }
      
      ReleaseDC(hdlg, hdc);

      return FALSE;
    }
    break;

  case WM_INITDIALOG:
    {
      _this=(mainWin *)lparam;
      SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)_this);

      _this->roi_w=512;
      _this->roi_h=512;
      _this->roi_b=32;
      _this->alloc_bufGDI(_this->roi_w, _this->roi_h, _this->roi_b);

      return TRUE;
    }
    break;
  }
  return FALSE;
}



