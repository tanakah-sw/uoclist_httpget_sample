// mainwin.h

#include <tchar.h>
#pragma warning(disable: 4996) // warning safe_stdio
#include <stdio.h>
#define _USE_MATH_DEFINES // use M_PI
#include <math.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // Vista=0x0600, 7=0x0601
#endif
#include <windows.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
/*
#pragma comment(linker, "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "\
    "processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
*/

#pragma warning(disable: 4458)
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

// STL
#include <vector>
#include <queue>
#include <map>

// utility
#include "util.h"

// logdisp
#include "logdisp.h"

#include "../httpget/httpget.h"

#pragma warning(disable: 4100)
#pragma warning(disable: 4102)

// global-variables
#ifndef DEFINE_GLOBAL_VARIABLE_HERE
extern HINSTANCE hAppInstance;
extern CRITICAL_SECTION c_section;
extern char homeFolderName[MAX_PATH];
#endif

#define MAINWIN_CLASS_NAME "mymainwin"
#define M_WIDTH 480
#define M_HEIGHT 480

class mainWin
{
public:
  mainWin(HINSTANCE hAppInstance);
  ~mainWin();
  static LRESULT CALLBACK wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

  void nonPassiveRefreshImage();

public:
  HWND m_hwnd, m_hdlg;

protected:
  HINSTANCE m_hAppInstance;
  void registerClass();
  void unregisterClass();

  GetEventCal geteventcal;

  void alloc_bufGDI(int width, int scanline, int bitcount);

  int roi_w, roi_h, roi_b;
  HBITMAP hbmp; unsigned char *m_buffer;
  PBITMAPINFO pbi;

  char iniFileName[MAX_PATH];
};

