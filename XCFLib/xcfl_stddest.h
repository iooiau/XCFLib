/******************************************************************************
*                                                                             *
*    xcfl_stddest.h                         Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_STD_DEST_H
#define XCFL_STD_DEST_H


#include "xcfl_destination.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflMemoryDest_tag xcflMemoryDest;
typedef struct {
	const void *pData;
	xcflSize Size;
} xcflMemoryDestBuffer;

XCFL_EXPORT(xcflError) xcflMemoryDest_Create(xcflMemoryDest **ppMemoryDest);
XCFL_EXPORT(xcflError) xcflMemoryDest_Delete(xcflMemoryDest **ppMemoryDest);
XCFL_EXPORT(xcflDestination*) xcflMemoryDest_GetDestination(const xcflMemoryDest *pMemoryDest);
XCFL_EXPORT(xcflError) xcflMemoryDest_CreateSource(const xcflMemoryDest *pMemoryDest,
												   xcflSource **ppSource);
XCFL_EXPORT(xcflError) xcflMemoryDest_GetBuffer(const xcflMemoryDest *pMemoryDest,
												xcflMemoryDestBuffer *pBuffer);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_STD_DEST_H */
