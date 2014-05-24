/******************************************************************************
*                                                                             *
*    xcfl_pixelbuffer.h                     Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_PIXEL_BUFFER_H
#define XCFL_PIXEL_BUFFER_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflPixelBuffer_tag xcflPixelBuffer;

typedef struct {
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt BytesPerPixel;
} xcflPixelBufferInfo;


XCFL_EXPORT(xcflError) xcflPixelBuffer_Create(xcflPixelBuffer **ppPixelBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_Delete(xcflPixelBuffer **ppPixelBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_Free(xcflPixelBuffer *pPixelBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_Allocate(xcflPixelBuffer *pPixelBuffer,
					xcflUInt32 Width,xcflUInt32 Height,xcflUInt BytesPerPixel);
XCFL_EXPORT(xcflError) xcflPixelBuffer_SetBuffer(xcflPixelBuffer *pPixelBuffer,
	xcflUInt32 Width,xcflUInt32 Height,xcflUInt BytesPerPixel,
	xcflInt RowStride,void *pBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_GetInfo(const xcflPixelBuffer *pPixelBuffer,
											   xcflPixelBufferInfo *pInfo);
XCFL_EXPORT(void*) xcflPixelBuffer_GetRow(const xcflPixelBuffer *pPixelBuffer,xcflUInt Top);
XCFL_EXPORT(void*) xcflPixelBuffer_GetPixelPtr(const xcflPixelBuffer *pPixelBuffer,
											   xcflUInt Left,xcflUInt Top);
XCFL_EXPORT(xcflError) xcflPixelBuffer_ClearPixels(xcflPixelBuffer *pPixelBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_Fill(xcflPixelBuffer *pPixelBuffer,
											const xcflUInt8 *pColor);
XCFL_EXPORT(xcflError) xcflPixelBuffer_SwapRGBOrder(xcflPixelBuffer *pPixelBuffer);
XCFL_EXPORT(xcflError) xcflPixelBuffer_Matte(xcflPixelBuffer *pPixelBuffer,
											 const xcflUInt8 *pBackColor);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_PIXEL_BUFFER_H */
