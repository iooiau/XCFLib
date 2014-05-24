/******************************************************************************
*                                                                             *
*    xcfl_property.h                        Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_PROPERTY_H
#define XCFL_PROPERTY_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	xcflUInt32 Type;
	xcflUInt32 Size;
	void *pData;
} xcflProperty;

typedef struct xcflPropertyList_tag xcflPropertyList;

#define XCFL_PROPERTY_END					0
#define XCFL_PROPERTY_COLORMAP				1
#define XCFL_PROPERTY_ACTIVE_LAYER			2
#define XCFL_PROPERTY_ACTIVE_CHANNEL		3
#define XCFL_PROPERTY_SELECTION				4
#define XCFL_PROPERTY_FLOATING_SELECTION	5
#define XCFL_PROPERTY_OPACITY				6
#define XCFL_PROPERTY_MODE					7
#define XCFL_PROPERTY_VISIBLE				8
#define XCFL_PROPERTY_LINKED				9
#define XCFL_PROPERTY_PRESERVE_TRANSPARENCY	10
#define XCFL_PROPERTY_APPLY_MASK			11
#define XCFL_PROPERTY_EDIT_MASK				12
#define XCFL_PROPERTY_SHOW_MASK				13
#define XCFL_PROPERTY_SHOW_MASKED			14
#define XCFL_PROPERTY_OFFSETS				15
#define XCFL_PROPERTY_COLOR					16
#define XCFL_PROPERTY_COMPRESSION			17
#define XCFL_PROPERTY_GUIDES				18
#define XCFL_PROPERTY_RESOLUTION			19
#define XCFL_PROPERTY_TATTOO				20
#define XCFL_PROPERTY_PARASITES				21
#define XCFL_PROPERTY_UNIT					22
#define XCFL_PROPERTY_PATHS					23
#define XCFL_PROPERTY_USER_UNIT				24
#define XCFL_PROPERTY_VECTORS				25
#define XCFL_PROPERTY_TEXT_LAYER_FLAGS		26
#define XCFL_PROPERTY_SAMPLE_POINTS			27
#define XCFL_PROPERTY_LOCK_CONTENT			28
#define XCFL_PROPERTY_GROUP_ITEM			29
#define XCFL_PROPERTY_ITEM_PATH				30
#define XCFL_PROPERTY_GROUP_ITEM_FLAGS		31


XCFL_EXPORT(xcflError) xcflProperty_Init(xcflProperty *pProperty);
XCFL_EXPORT(xcflError) xcflProperty_Free(xcflProperty *pProperty);
XCFL_EXPORT(xcflError) xcflProperty_Copy(xcflProperty *pProperty,
										 const xcflProperty *pSrcProperty);
XCFL_EXPORT(xcflUInt32) xcflProperty_GetUInt32(const xcflProperty *pProperty,
											   int Index);
XCFL_EXPORT(xcflInt32) xcflProperty_GetInt32(const xcflProperty *pProperty,
											 int Index);
XCFL_EXPORT(xcflFloat32) xcflProperty_GetFloat(const xcflProperty *pProperty,
											   int Index);

XCFL_EXPORT(xcflError) xcflPropertyList_Create(xcflPropertyList **ppPropertyList);
XCFL_EXPORT(xcflError) xcflPropertyList_Delete(xcflPropertyList **ppPropertyList);
XCFL_EXPORT(xcflError) xcflPropertyList_Clear(xcflPropertyList *pPropertyList);
XCFL_EXPORT(xcflError) xcflPropertyList_Read(xcflPropertyList *pPropertyList,
											 xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflPropertyList_SetProperty(
					xcflPropertyList *pPropertyList,xcflProperty *pProperty);
XCFL_EXPORT(xcflError) xcflPropertyList_RemoveProperty(
							xcflPropertyList *pPropertyList,xcflUInt32 Type);
XCFL_EXPORT(xcflBool) xcflPropertyList_GetProperty(
				const xcflPropertyList *pPropertyList,xcflProperty *pProperty);
XCFL_EXPORT(xcflBool) xcflPropertyList_GetPropertyByIndex(
	const xcflPropertyList *pPropertyList,int Index,xcflProperty *pProperty);
XCFL_EXPORT(xcflBool) xcflPropertyList_HasProperty(
						const xcflPropertyList *pPropertyList,xcflUInt32 Type);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_PROPERTY_H */
