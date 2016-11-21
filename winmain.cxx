// winmain.cxx

#define DEFINE_GLOBAL_VARIABLE_HERE
#include "mainwin.h"

HINSTANCE hAppInstance;
CRITICAL_SECTION c_section;
char homeFolderName[MAX_PATH];

#define LOOPDURATION 33 // 33msec Loop -> 30FPS

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  INITCOMMONCONTROLSEX icex;
  icex.dwSize=sizeof(INITCOMMONCONTROLSEX); icex.dwICC=ICC_COOL_CLASSES|ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icex);

  LoadLibrary("RichEd32.dll");
  OleInitialize(NULL);

  //  AVIFileInit();
  ULONG_PTR gdiplus_token;
  GdiplusStartupInput gsi; GdiplusStartup(&gdiplus_token, &gsi, NULL);
  timeBeginPeriod(1);

  hAppInstance=hInstance;
  InitializeCriticalSection(&c_section);

  GetModuleFileName(NULL, homeFolderName, MAX_PATH);
  PathRemoveFileSpec(homeFolderName);

  mainWin *mainwin=new mainWin(hInstance);

  MSG msg;
  DWORD next=timeGetTime()+LOOPDURATION;
  bool done=false;
  unsigned int counter=0;

  while(!done){
    if(PeekMessage(&msg,0, 0, 0, PM_REMOVE)){
      if(msg.message==WM_QUIT){
        done=TRUE;
      }else{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }else{
      DWORD now=timeGetTime();
      if(now>=next){
        // proc
      procend:
        mainwin->nonPassiveRefreshImage();
        next=now+LOOPDURATION;
        counter++;
      }
      int diff=next-timeGetTime();
      if(diff>0){
        MsgWaitForMultipleObjects(0, NULL, FALSE, diff, QS_ALLINPUT);
      }
    }
  }

 winend:
  delete mainwin;

  DeleteCriticalSection(&c_section);
  timeEndPeriod(1);
  GdiplusShutdown(gdiplus_token);
  //  AVIFileExit();
  OleUninitialize();

  return 0;
}
