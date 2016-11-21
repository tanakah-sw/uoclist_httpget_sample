// tips.h

// BMP -------------------------------------------------------------------------------------
#ifndef SIZEOFBITMAPINFO
#define SIZEOFBITMAPINFO (sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256+sizeof(DWORD)*2)
#endif

// DWM -------------------------------------------------------------------------------------
#ifndef DWMWA_EXTENDED_FRAME_BOUNDS
#define DWMWA_EXTENDED_FRAME_BOUNDS 0x09
#endif

inline void MyGetWindowRect(HWND hwnd, RECT *rect)
{
  static HRESULT (WINAPI *DwmIsCompositionEnabled)(BOOL *)=NULL;
  static HRESULT (WINAPI *DwmGetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD)=NULL;
  BOOL bAero=FALSE;

  if((DwmIsCompositionEnabled==NULL)||(DwmGetWindowAttribute==NULL)){
    HMODULE hdll=LoadLibrary("dwmapi");
    if(hdll){
      DwmIsCompositionEnabled=(HRESULT(WINAPI *)(BOOL *))
        GetProcAddress(hdll, "DwmIsCompositionEnabled");  
      DwmGetWindowAttribute=(HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD))
        GetProcAddress(hdll, "DwmGetWindowAttribute");
    }
  }
  
  if(DwmIsCompositionEnabled!=NULL){
    DwmIsCompositionEnabled(&bAero);
  }
  if(bAero==FALSE){
    GetWindowRect(hwnd, rect);
    return;
  }else{
    if(DwmGetWindowAttribute!=NULL){
      DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(RECT));
    }
    return;
  }
}

// GDI+ ------------------------------------------------------------------------------------
inline int GetEncoderCLSID(const WCHAR *format, CLSID *pclsid)
{
  unsigned int num=0, size=0;
  Gdiplus::GetImageEncodersSize(&num, &size);
  if(size==0) return -1;
  Gdiplus::ImageCodecInfo *pImageCodecInfo=(Gdiplus::ImageCodecInfo *)malloc(size);
  if(pImageCodecInfo==NULL) return -1;
  Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
  for(unsigned int j=0; j<num; j++){
    if(wcscmp(pImageCodecInfo[j].MimeType, format)==0){
      *pclsid=pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;
    }
  }
  free(pImageCodecInfo);
  return -1;
}

inline void saveAsPngJpeg(char *filename, unsigned char *s_image, int w, int h, int b)
{
  char *lpe=PathFindExtension(filename);
  wchar_t mimecode[16];
  if(strlen(lpe)<5){
    if(stricmp(lpe, ".png")==0){
      wcscpy(mimecode, L"image/png");
    } else if((stricmp(lpe, ".jpg")==0)||(stricmp(lpe, ".jpeg")==0)){
      wcscpy(mimecode, L"image/jpeg");
    }else{
      return;
    }
  }

  DeleteFile(filename);
  WCHAR wfilename[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, MAX_PATH);

  PBITMAPINFO pbiGDI=(PBITMAPINFO)malloc(SIZEOFBITMAPINFO);
  memset(pbiGDI, 0, SIZEOFBITMAPINFO);
  
  BITMAPINFOHEADER bi;
  bi.biSize=sizeof(BITMAPINFOHEADER);
  bi.biWidth=w;
  bi.biHeight=-h; // 上下反転
  bi.biPlanes=1;
  bi.biXPelsPerMeter=bi.biYPelsPerMeter=0;

  // モノクロ想定
  if((b==8)||(b==10)||(b==12)||(b==14)||(b==16)){
    bi.biBitCount=8;
    bi.biCompression=BI_RGB;
    bi.biSizeImage=h*((w*bi.biBitCount+31)/32*4); // 4バイトalign
    //    bi.biSizeImage=h*w;
    bi.biClrUsed=bi.biClrImportant=0;
  }else{
    bi.biBitCount=(WORD)b;
    bi.biCompression=BI_RGB;
    bi.biSizeImage=h*((w*bi.biBitCount+31)/32*4); // 4バイトalign
    bi.biClrUsed=bi.biClrImportant=0;
  }

  memcpy(&(pbiGDI->bmiHeader), &bi, sizeof(BITMAPINFOHEADER));

  // paletteの設定
  switch(bi.biBitCount){
  case 8: // モノクロ固定
    {
      for(int c=0; c<256; c++){
        pbiGDI->bmiColors[c].rgbBlue=(c&0xff);
        pbiGDI->bmiColors[c].rgbGreen=(c&0xff);
        pbiGDI->bmiColors[c].rgbRed=(c&0xff);
      }
    }
    break;
  case 10: // 出力として使用しないがコードとして残す
    {
      for(int c=0; c<3; c++){
        pbiGDI->bmiColors[c].rgbBlue=0xff;
        pbiGDI->bmiColors[c].rgbGreen=0x03;
        pbiGDI->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  case 12: // 出力として使用しないがコードとして残す
    {
      for(int c=0; c<3; c++){
        pbiGDI->bmiColors[c].rgbBlue=0xff;
        pbiGDI->bmiColors[c].rgbGreen=0x0f;
        pbiGDI->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  case 14: // 出力として使用しないがコードとして残す
    {
      for(int c=0; c<3; c++){
        pbiGDI->bmiColors[c].rgbBlue=0xff;
        pbiGDI->bmiColors[c].rgbGreen=0x3f;
        pbiGDI->bmiColors[c].rgbRed=0x00;
      }
    }
    break;
  }

  unsigned char *d_image;
  HBITMAP hbmpGDI=CreateDIBSection(NULL, pbiGDI, DIB_RGB_COLORS, (void **)(&d_image), NULL, 0);
  memset(d_image, 0, bi.biSizeImage);

  // 1Xbit->8bit decrease
  int sbl=(w*16+31)/32*4;
  int dbl=(w*8+31)/32*4;
  if(b==10){
    unsigned short *sp=(unsigned short *)s_image;
    unsigned char *dp=d_image;
    for(int i=0; i<h; i++){
      for(int j=0; j<w; j++){
        *dp=((*sp)>>2)&0xff;
        ++sp; ++dp;
      }
      // skip padding
      dp+=(dbl-w);
      sp+=(sbl-w*2);
    }
  }else if(b==12){
    unsigned short *sp=(unsigned short *)s_image;
    unsigned char *dp=d_image;
    for(int i=0; i<h; i++){
      for(int j=0; j<w; j++){
        *dp=((*sp)>>2)&0xff;
        ++sp; ++dp;
      }
      // skip padding
      dp+=(dbl-w);
      sp+=(sbl-w*2);
    }
  }else if(b==14){
    unsigned short *sp=(unsigned short *)s_image;
    unsigned char *dp=d_image;
    for(int i=0; i<h; i++){
      for(int j=0; j<w; j++){
        *dp=((*sp)>>2)&0xff;
        ++sp; ++dp;
      }
      // skip padding
      dp+=(dbl-w);
      sp+=(sbl-w*2);
    }
  }

  {
    Bitmap *pbmp=Bitmap::FromHBITMAP(hbmpGDI, NULL);
    
    CLSID clsid; GetEncoderCLSID(mimecode, &clsid);
    // for jpeg
    EncoderParameters epara;
    epara.Count=1;
    epara.Parameter[0].Guid=EncoderQuality;
    epara.Parameter[0].Type=EncoderParameterValueTypeLong;
    epara.Parameter[0].NumberOfValues=1;
    int quality=80;
    epara.Parameter[0].Value=&quality;
    
    pbmp->Save(wfilename, &clsid, &epara);
    
    delete pbmp;
  }
  
  DeleteObject(hbmpGDI);
  free(pbiGDI);

}

inline int bpl24(int width)
{
  return (width*24+31)/32*4;
}

inline int mgetshort(unsigned char *mp)
{
  int c1, c2;
  c1=*(mp);
  c2=*(mp+1);
  return (c1<<8)+(c2);
}

inline int mgetint(unsigned char *mp)
{
  int c1, c2, c3, c4;
  c1=*(mp);
  c2=*(mp+1);
  c3=*(mp+2);
  c4=*(mp+3);
  return (c1<<24)+(c2<<16)+(c3<<8)+c4;
}

inline int mgetshortLE(unsigned char *mp)
{
  int c1, c2;
  c1=*(mp);
  c2=*(mp+1);
  return (c2<<8)+(c1);
}

inline int mgetintLE(unsigned char *mp)
{
  int c1, c2, c3, c4;
  c1=*(mp+3);
  c2=*(mp+2);
  c3=*(mp+1);
  c4=*(mp+0);
  return (c1<<24)+(c2<<16)+(c3<<8)+c4;
}

inline void mputshort(unsigned char *p, unsigned short i)
{
  *(p+0)=HIBYTE(i);
  *(p+1)=LOBYTE(i);
  return;
}

inline void mputint(unsigned char *p, unsigned int i)
{
  *(p+0)=HIBYTE(HIWORD(i));
  *(p+1)=LOBYTE(HIWORD(i));
  *(p+2)=HIBYTE(LOWORD(i));
  *(p+3)=LOBYTE(LOWORD(i));
}

inline char halfbyte2ascii(char c1)
{
  char c2;
  if(c1>=10) c2='A'+(c1-10); else c2='0'+c1;
  return c2;
}

inline void byte2ascii(char *a, unsigned char c)
{
  a[0]=halfbyte2ascii((c&0xf0)>>4);
  a[1]=halfbyte2ascii(c&0x0f);
}

inline unsigned char ascii2byte(char c1, char c2)
{
  unsigned char uc1=0, uc2=0;
  if((c1>='A')&&(c1<='F')) uc1=0x0a+(c1-'A');
  else if((c1>='a')&&(c1<='f')) uc1=0x0a+(c1-'a');
  else uc1=c1-'0';
  
  if((c2>='A')&&(c2<='F')) uc2=0x0a+(c2-'A');
  else if((c2>='a')&&(c2<='f')) uc1=0x0a+(c2-'a');
  else uc2=c2-'0';

  return uc1<<4|uc2;
}

inline void ascii2binary(unsigned char *dst, unsigned char *src, int len)
{
  for(int i=0; i<len; i++){
    *(dst+i)=ascii2byte(*(src+i*2+0), *(src+i*2+1));
  }
}

inline void binary2ascii(char *dst, unsigned char *src, int len)
{
  for(int i=0; i<len; i++){
    byte2ascii(dst+i*2, *(src+i));
  }
}

inline void calcFCS(char *data, char fcschar[2], int len)
{
  unsigned char fcs=*data;
  for(int i=1; i<len; i++){
    fcs^=*(data+i);
  }
  fcschar[0]=halfbyte2ascii((fcs&0xf0)>>4);
  fcschar[1]=halfbyte2ascii(fcs&0x0f);
}
