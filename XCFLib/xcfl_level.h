/******************************************************************************
*                                                                             *
*    xcfl_level.h                           Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_LEVEL_H
#define XCFL_LEVEL_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflLevel_tag xcflLevel;

typedef struct {
	xcflUInt32 Width;
	xcflUInt32 Height;
} xcflLevelInfo;


XCFL_EXPORT(xcflError) xcflLevel_Create(xcflLevel **ppLevel,
										xcflUInt32 BytesPerPixel,
										xcflCompression Compression);
XCFL_EXPORT(xcflError) xcflLevel_Delete(xcflLevel **ppLevel);
XCFL_EXPORT(xcflError) xcflLevel_ReadHeader(xcflLevel *pLevel,xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflLevel_GetInfo(const xcflLevel *pLevel,xcflLevelInfo *pInfo);
XCFL_EXPORT(xcflError) xcflLevel_ReadData(xcflLevel *pLevel,xcflSource *pSource);
XCFL_EXPORT(xcflBool) xcflLevel_IsDataLoaded(const xcflLevel *pLevel);
XCFL_EXPORT(void*) xcflLevel_GetData(const xcflLevel *pLevel);
XCFL_EXPORT(void*) xcflLevel_GetRow(const xcflLevel *pLevel,xcflUInt Top);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_LEVEL_H */
