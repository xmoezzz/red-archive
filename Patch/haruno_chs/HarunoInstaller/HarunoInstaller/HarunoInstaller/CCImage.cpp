#include "stdafx.h"
#include "CCImage.h"

HRESULT CCImage::Load(IStream* pStream, bool bPreMultiply)
{
 HRESULT retRes = CImage::Load(pStream);
 if(!IsNull() && bPreMultiply)
 {
  unsigned char * pCol = 0;
  long lW = GetWidth();
  long lH = GetHeight();
  for(long ixy = 0; ixy < lH; ixy ++)
  {
   for(long ixx = 0; ixx < lW; ixx ++)
   {
    pCol = (unsigned char *)GetPixelAddress(ixx,ixy);
    unsigned char alpha = pCol[3];
    if(alpha < 255)
    {
    pCol[0] = ((pCol[0] * alpha) + 127) / 255;
    pCol[1] = ((pCol[1] * alpha) + 127) / 255;
    pCol[2] = ((pCol[2] * alpha) + 127) / 255;
    }
   }
  }
 }
 return retRes;
}

