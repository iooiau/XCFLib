/******************************************************************************
*                                                                             *
*    xcfl_layer.h                           Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_LAYER_H
#define XCFL_LAYER_H


#include "xcfl_item.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflLayer_tag xcflLayer;

typedef struct {
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflImageType Type;
	xcflCompositeMode Mode;
	xcflUInt32 Opacity;
	xcflInt32 XOffset;
	xcflInt32 YOffset;
	xcflUInt Flags;
} xcflLayerInfo;

#define XCFL_LAYER_FLAG_VISIBLE					0x0001
#define XCFL_LAYER_FLAG_ACTIVE					0x0002
#define XCFL_LAYER_FLAG_PRESERVE_TRANSPARENCY	0x0004
#define XCFL_LAYER_FLAG_HAS_MASK				0x0008
#define XCFL_LAYER_FLAG_APPLY_MASK				0x0010
#define XCFL_LAYER_FLAG_EDIT_MASK				0x0020
#define XCFL_LAYER_FLAG_SHOW_MASK				0x0040
#define XCFL_LAYER_FLAG_GROUP					0x0080


XCFL_EXPORT(xcflError) xcflLayer_Create(xcflLayer **ppLayer);
XCFL_EXPORT(xcflError) xcflLayer_Delete(xcflLayer **ppLayer);
XCFL_EXPORT(xcflError) xcflLayer_DeleteAllSiblings(xcflLayer **ppLayer);
XCFL_EXPORT(xcflError) xcflLayer_ReadHeader(xcflLayer *pLayer,
											xcflSource *pSource);
XCFL_EXPORT(xcflBool) xcflLayer_GetProperty(const xcflLayer *pLayer,
											xcflProperty *pProperty);
XCFL_EXPORT(xcflError) xcflLayer_GetInfo(const xcflLayer *pLayer,
										 xcflLayerInfo *pInfo);
XCFL_EXPORT(const char*) xcflLayer_GetName(const xcflLayer *pLayer);
XCFL_EXPORT(xcflError) xcflLayer_ReadData(xcflLayer *pLayer,
										  xcflSource *pSource,
										  xcflCompression Compression);
XCFL_EXPORT(xcflBool) xcflLayer_IsDataLoaded(const xcflLayer *pLayer);
XCFL_EXPORT(xcflError) xcflLayer_ReadMask(xcflLayer *pLayer,
										  xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflLayer_GetMaskInfo(const xcflLayer *pLayer,
											 xcflChannelInfo *pInfo);
XCFL_EXPORT(xcflError) xcflLayer_ReadMaskData(xcflLayer *pLayer,
											  xcflSource *pSource,
											  xcflCompression Compression);
XCFL_EXPORT(xcflError) xcflLayer_CompositePixels(xcflLayer *pLayer,
												 xcflPixelBuffer *pPixelBuffer,
												 const xcflPoint *pSrcPos);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetPrevSibling(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetNextSibling(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetFirstSibling(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetLastSibling(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetSibling(const xcflLayer *pLayer,
											 int Index);
XCFL_EXPORT(xcflError) xcflLayer_AppendSibling(xcflLayer *pLayer,
											   xcflLayer *pSibling);
XCFL_EXPORT(xcflBool) xcflLayer_IsGroup(const xcflLayer *pLayer);
XCFL_EXPORT(xcflBool) xcflLayer_HasChild(const xcflLayer *pLayer);
XCFL_EXPORT(int) xcflLayer_GetChildCount(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetFirstChild(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetLastChild(const xcflLayer *pLayer);
XCFL_EXPORT(xcflLayer*) xcflLayer_GetChild(const xcflLayer *pLayer,
										   int Index);
XCFL_EXPORT(xcflError) xcflLayer_AppendChild(xcflLayer *pLayer,
											 xcflLayer *pChild);
XCFL_EXPORT(xcflBool) xcflLayer_HasItemPath(const xcflLayer *pLayer);
XCFL_EXPORT(xcflError) xcflLayer_GetItemPath(const xcflLayer *pLayer,
											 xcflItemPath *pPath);

#define xcflLayer_GetUpperSibling	xcflLayer_GetPrevSibling
#define xcflLayer_GetLowerSibling	xcflLayer_GetLowerSibling
#define xcflLayer_GetTopSibling		xcflLayer_GetFirstSibling
#define xcflLayer_GetBottomSibling	xcflLayer_GetLastSibling
#define xcflLayer_GetTopChild		xcflLayer_GetFirstChild
#define xcflLayer_GetBottomChild	xcflLayer_GetLastChild


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_LAYER_H */
