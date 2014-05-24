/******************************************************************************
*                                                                             *
*    xcfl_item.h                            Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_ITEM_H
#define XCFL_ITEM_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflItemPath_tag {
	xcflUInt NumElements;
	xcflUInt32 *pList;
} xcflItemPath;


xcflError xcflItemPath_Init(xcflItemPath *pPath);
xcflError xcflItemPath_Free(xcflItemPath *pPath);
xcflError xcflItemPath_Allocate(xcflItemPath *pPath,xcflUInt NumElements);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_IMAGE_H */
