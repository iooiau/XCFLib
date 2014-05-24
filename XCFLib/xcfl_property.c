/******************************************************************************
*                                                                             *
*    xcfl_property.c                        Copyright(c) 2010-2013 itow,y.    *
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


#define XCFL_INTERNALS
#include "xcfl.h"
#include "xcfl_util.h"


struct xcflPropertyList_tag {
	xcflProperty *pList;
	int ListLength;
	int NumProperties;
};




XCFL_EXPORT(xcflError) xcflProperty_Init(xcflProperty *pProperty)
{
	if (pProperty==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pProperty->Type=0;
	pProperty->Size=0;
	pProperty->pData=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflProperty_Free(xcflProperty *pProperty)
{
	if (pProperty==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflMemoryFree(pProperty->pData);
	xcflProperty_Init(pProperty);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflProperty_Copy(xcflProperty *pProperty,
										 const xcflProperty *pSrcProperty)
{
	if (pProperty==NULL || pSrcProperty==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pProperty->Type=pSrcProperty->Type;
	pProperty->Size=pSrcProperty->Size;
	pProperty->pData=xcflMemoryDuplicate(pSrcProperty->pData,pSrcProperty->Size);
	if (pProperty->pData==NULL) {
		xcflProperty_Init(pProperty);
		return XCFL_ERR_MEMORY_ALLOC;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflUInt32) xcflProperty_GetUInt32(const xcflProperty *pProperty,
											   int Index)
{
	if (pProperty==NULL || Index<0 || (xcflSize)Index>=pProperty->Size/4)
		return 0;

	return xcflGetMSBFirst32((const xcflUInt8*)pProperty->pData+Index*4);
}


XCFL_EXPORT(xcflInt32) xcflProperty_GetInt32(const xcflProperty *pProperty,
											 int Index)
{
	if (pProperty==NULL || Index<0 || (xcflSize)Index>=pProperty->Size/4)
		return 0;

	return (xcflInt32)xcflGetMSBFirst32((const xcflUInt8*)pProperty->pData+Index*4);
}


XCFL_EXPORT(xcflFloat32) xcflProperty_GetFloat(const xcflProperty *pProperty,
											   int Index)
{
	xcflUInt32 Value;

	if (pProperty==NULL || Index<0 || (xcflSize)Index>=pProperty->Size/4)
		return 0;

	Value=xcflGetMSBFirst32((const xcflUInt8*)pProperty->pData+Index*4);

	return *(xcflFloat32*)&Value;
}




static int FindProperty(const xcflPropertyList *pPropertyList,xcflUInt32 Type)
{
	int i;

	for (i=0;i<pPropertyList->NumProperties;i++) {
		if (pPropertyList->pList[i].Type==Type)
			return i;
	}

	return -1;
}


XCFL_EXPORT(xcflError) xcflPropertyList_Create(xcflPropertyList **ppPropertyList)
{
	xcflPropertyList *pPropertyList;

	if (ppPropertyList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppPropertyList=xcflNew(xcflPropertyList);
	if (*ppPropertyList==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pPropertyList=*ppPropertyList;

	pPropertyList->pList=NULL;
	pPropertyList->ListLength=0;
	pPropertyList->NumProperties=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPropertyList_Delete(xcflPropertyList **ppPropertyList)
{
	if (ppPropertyList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppPropertyList!=NULL) {
		xcflPropertyList_Clear(*ppPropertyList);
		xcflDelete(*ppPropertyList);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPropertyList_Clear(xcflPropertyList *pPropertyList)
{
	if (pPropertyList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPropertyList->pList!=NULL) {
		int i;

		for (i=pPropertyList->NumProperties-1;i>=0;i--)
			xcflProperty_Free(&pPropertyList->pList[i]);
		xcflMemoryFree(pPropertyList->pList);
		pPropertyList->pList=NULL;
		pPropertyList->ListLength=0;
	}
	pPropertyList->NumProperties=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPropertyList_Read(xcflPropertyList *pPropertyList,
											 xcflSource *pSource)
{
	while (XCFL_TRUE) {
		xcflProperty Property;
		xcflError Err;

		if (!xcflSource_ReadUInt32(pSource,&Property.Type)
				|| !xcflSource_ReadUInt32(pSource,&Property.Size))
			return XCFL_ERR_READ;

		if (Property.Type==XCFL_PROPERTY_END) {
			if (Property.Size!=0)
				return XCFL_ERR_BAD_FORMAT;
			break;
		}

		if (Property.Type==XCFL_PROPERTY_COLORMAP) {
			xcflUInt32 NumColors;
			xcflUInt8 *p;

			if (Property.Size<4)
				return XCFL_ERR_BAD_FORMAT;

			if (!xcflSource_ReadUInt32(pSource,&NumColors))
				return XCFL_ERR_READ;

			if (NumColors>256)
				return XCFL_ERR_BAD_FORMAT;

			if (Property.Size==4+NumColors)
				Property.Size=4+3*NumColors;
			else if (4+NumColors*3>Property.Size)
				return XCFL_ERR_BAD_FORMAT;

			Property.pData=xcflMemoryAlloc(Property.Size);
			if (Property.pData==NULL)
				return XCFL_ERR_MEMORY_ALLOC;

			p=(xcflUInt8*)Property.pData;

			p[0]=(xcflUInt8)(NumColors>>24);
			p[1]=(xcflUInt8)((NumColors>>16)&0xFF);
			p[2]=(xcflUInt8)((NumColors>>8)&0xFF);
			p[3]=(xcflUInt8)(NumColors&0xFF);

			if (xcflSource_Read(pSource,&p[4],Property.Size-4)!=Property.Size-4) {
				xcflProperty_Free(&Property);
				return XCFL_ERR_READ;
			}
		} else if (Property.Size>0) {
			Property.pData=xcflMemoryAlloc(Property.Size);
			if (Property.pData==NULL)
				return XCFL_ERR_MEMORY_ALLOC;

			if (xcflSource_Read(pSource,Property.pData,Property.Size)!=
															Property.Size) {
				xcflProperty_Free(&Property);
				return XCFL_ERR_READ;
			}
		} else {
			Property.pData=NULL;
		}

		Err=xcflPropertyList_SetProperty(pPropertyList,&Property);
		if (Err!=XCFL_ERR_SUCCESS) {
			xcflProperty_Free(&Property);
			return Err;
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPropertyList_SetProperty(
					xcflPropertyList *pPropertyList,xcflProperty *pProperty)
{
	int i;

	if (pPropertyList==NULL || pProperty==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	i=FindProperty(pPropertyList,pProperty->Type);

	if (i>=0) {
		xcflProperty_Free(&pPropertyList->pList[i]);
	} else {
		if (pPropertyList->NumProperties==pPropertyList->ListLength) {
			int NewLength;
			xcflProperty *pNewList;

			if (pPropertyList->ListLength==0)
				NewLength=8;
			else
				NewLength=pPropertyList->ListLength*2;
			pNewList=(xcflProperty*)xcflMemoryReAlloc(pPropertyList->pList,
													  NewLength*sizeof(xcflProperty));
			if (pNewList==NULL)
				return XCFL_ERR_MEMORY_ALLOC;

			pPropertyList->pList=pNewList;
			pPropertyList->ListLength=NewLength;
		}

		i=pPropertyList->NumProperties;
		pPropertyList->NumProperties++;
	}

	pPropertyList->pList[i]=*pProperty;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPropertyList_RemoveProperty(
							xcflPropertyList *pPropertyList,xcflUInt32 Type)
{
	int i;

	if (pPropertyList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	i=FindProperty(pPropertyList,Type);
	if (i<0)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflProperty_Free(&pPropertyList->pList[i]);
	pPropertyList->NumProperties--;
	if (i<pPropertyList->NumProperties)
		xcflMemoryMove(&pPropertyList->pList[i],
					   &pPropertyList->pList[i+1],
					   (pPropertyList->NumProperties-i)*sizeof(xcflProperty));

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflPropertyList_GetProperty(
				const xcflPropertyList *pPropertyList,xcflProperty *pProperty)
{
	int i;

	if (pPropertyList==NULL || pProperty==NULL)
		return XCFL_FALSE;

	i=FindProperty(pPropertyList,pProperty->Type);
	if (i<0)
		return XCFL_FALSE;

	*pProperty=pPropertyList->pList[i];

	return XCFL_TRUE;
}


XCFL_EXPORT(xcflBool) xcflPropertyList_GetPropertyByIndex(
	const xcflPropertyList *pPropertyList,int Index,xcflProperty *pProperty)
{
	if (pPropertyList==NULL || pProperty==NULL
			|| Index<0 || Index>=pPropertyList->NumProperties)
		return XCFL_FALSE;

	*pProperty=pPropertyList->pList[Index];

	return XCFL_FALSE;
}


XCFL_EXPORT(xcflBool) xcflPropertyList_HasProperty(
						const xcflPropertyList *pPropertyList,xcflUInt32 Type)
{
	if (pPropertyList==NULL)
		return XCFL_FALSE;

	return FindProperty(pPropertyList,Type)>=0;
}
