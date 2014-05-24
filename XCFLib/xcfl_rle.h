/******************************************************************************
*                                                                             *
*    xcfl_rle.h                             Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_RLE_H
#define XCFL_RLE_H


#ifdef __cplusplus
extern "C" {
#endif


xcflError xcflRLEDecode(xcflBufferedSource *pSource,
						xcflUInt32 Width,xcflUInt32 Height,xcflUInt32 BytesPerPixel,
						void *pDstData);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_RLE_H */
