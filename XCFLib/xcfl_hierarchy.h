/******************************************************************************
*                                                                             *
*    xcfl_hierarchy.h                       Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_HIERARCHY_H
#define XCFL_HIERARCHY_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflHierarchy_tag xcflHierarchy;

typedef struct {
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt32 BytesPerPixel;
} xcflHierarchyInfo;


XCFL_EXPORT(xcflError) xcflHierarchy_Create(xcflHierarchy **ppHierarchy,
											xcflCompression Compression);
XCFL_EXPORT(xcflError) xcflHierarchy_Delete(xcflHierarchy **ppHierarchy);
XCFL_EXPORT(xcflError) xcflHierarchy_ReadHeader(xcflHierarchy *pHierarchy,
												xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflHierarchy_GetInfo(const xcflHierarchy *pHierarchy,
											 xcflHierarchyInfo *pInfo);
XCFL_EXPORT(xcflError) xcflHierarchy_ReadData(xcflHierarchy *pHierarchy,
											  xcflSource *pSource);
XCFL_EXPORT(xcflBool) xcflHierarchy_IsDataLoaded(const xcflHierarchy *pHierarchy);
XCFL_EXPORT(void*) xcflHierarchy_GetData(const xcflHierarchy *pHierarchy);
XCFL_EXPORT(void*) xcflHierarchy_GetRow(const xcflHierarchy *pHierarchy,xcflUInt Top);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_HIERARCHY_H */
