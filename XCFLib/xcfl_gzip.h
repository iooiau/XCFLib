/******************************************************************************
*                                                                             *
*    xcfl_gzip.h                            Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_GZIP_H
#define XCFL_GZIP_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflGZip_tag xcflGZip;

XCFL_EXPORT(xcflError) xcflGZip_Create(xcflGZip **ppGZip,xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflGZip_Delete(xcflGZip **ppGZip);
XCFL_EXPORT(xcflError) xcflGZip_ReadHeader(xcflGZip *pGZip);
XCFL_EXPORT(xcflError) xcflGZip_Decompress(xcflGZip *pGZip,
										   void *pBuffer,xcflSize *pSize);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_GZIP_H */
