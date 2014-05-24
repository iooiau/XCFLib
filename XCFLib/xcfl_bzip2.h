/******************************************************************************
*                                                                             *
*    xcfl_bzip2.h                           Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_BZIP2_H
#define XCFL_BZIP2_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflBZip2_tag xcflBZip2;

XCFL_EXPORT(xcflError) xcflBZip2_Create(xcflBZip2 **ppBZip2,xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflBZip2_Delete(xcflBZip2 **ppBZip2);
XCFL_EXPORT(xcflError) xcflBZip2_ReadHeader(xcflBZip2 *pBZip2);
XCFL_EXPORT(xcflError) xcflBZip2_Decompress(xcflBZip2 *pBZip2,
											void *pBuffer,xcflSize *pSize);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_BZIP2_H */
