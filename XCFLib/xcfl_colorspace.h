/******************************************************************************
*                                                                             *
*    xcfl_colorspace.h                      Copyright(c) 2010-2013 itow,y.    *
*                                                                             *
******************************************************************************/

/*
  XCFLib - GIMP XCF format library
  Copyright(c) 2010-2013 itow,y.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301, USA.
*/


#ifndef XCFL_COLORSPACE_H
#define XCFL_COLORSPACE_H


#ifdef __cplusplus
extern "C" {
#endif


XCFL_EXPORT(void) xcflRGBToHSVInt(int *pRed,int *pGreen,int *pBlue);
XCFL_EXPORT(void) xcflHSVToRGBInt(int *pHue,int *pSaturation,int *pValue);
XCFL_EXPORT(int) xcflRGBToHSVInt_S(int Red,int Green,int Blue);
#define xcflRGBToHSVInt_V(r,g,b) ((r)>(g)?xcflMax(r,b):xcflMax(g,b))
XCFL_EXPORT(void) xcflRGBToHSLInt(int *pRed,int *pGreen,int *pBlue);
XCFL_EXPORT(void) xcflHSLToRGBInt(int *pHue,int *pSaturation,int *pLightness);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_COLORSPACE_H */
