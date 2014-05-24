/******************************************************************************
*                                                                             *
*    xcfl_hierarchy.c                       Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_hierarchy.h"
#include "xcfl_level.h"


struct xcflHierarchy_tag {
	xcflUInt Status;
	xcflCompression Compression;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt32 BytesPerPixel;
	xcflUInt32 LevelOffset;
	xcflLevel *pLevel;
};

#define XCFL_HIERARCHY_STATUS_HEADER_READ	0x0001




XCFL_EXPORT(xcflError) xcflHierarchy_Create(xcflHierarchy **ppHierarchy,
											xcflCompression Compression)
{
	xcflHierarchy *pHierarchy;

	if (ppHierarchy==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppHierarchy=xcflNew(xcflHierarchy);
	if (*ppHierarchy==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pHierarchy=*ppHierarchy;

	pHierarchy->Status=0;
	pHierarchy->Compression=Compression;
	pHierarchy->Width=0;
	pHierarchy->Height=0;
	pHierarchy->BytesPerPixel=0;
	pHierarchy->LevelOffset=0;
	pHierarchy->pLevel=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflHierarchy_Delete(xcflHierarchy **ppHierarchy)
{
	if (ppHierarchy==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppHierarchy!=NULL) {
		xcflLevel_Delete(&(*ppHierarchy)->pLevel);

		xcflDelete(*ppHierarchy);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflHierarchy_ReadHeader(xcflHierarchy *pHierarchy,
												xcflSource *pSource)
{
	xcflUInt32 Width,Height,BytesPerPixel,LevelOffset;

	if (pHierarchy==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pHierarchy->Status&XCFL_HIERARCHY_STATUS_HEADER_READ)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (!xcflSource_ReadUInt32(pSource,&Width)
			|| !xcflSource_ReadUInt32(pSource,&Height)
			|| !xcflSource_ReadUInt32(pSource,&BytesPerPixel)
			|| !xcflSource_ReadUInt32(pSource,&LevelOffset))
		return XCFL_ERR_READ;

	if (Width==0 || Height==0 || BytesPerPixel==0)
		return XCFL_ERR_BAD_FORMAT;

	pHierarchy->Status|=XCFL_HIERARCHY_STATUS_HEADER_READ;

	pHierarchy->Width=Width;
	pHierarchy->Height=Height;
	pHierarchy->BytesPerPixel=BytesPerPixel;
	pHierarchy->LevelOffset=LevelOffset;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflHierarchy_GetInfo(const xcflHierarchy *pHierarchy,
											 xcflHierarchyInfo *pInfo)
{
	if (pHierarchy==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pInfo->Width=pHierarchy->Width;
	pInfo->Height=pHierarchy->Height;
	pInfo->BytesPerPixel=pHierarchy->BytesPerPixel;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflHierarchy_ReadData(xcflHierarchy *pHierarchy,
											  xcflSource *pSource)
{
	if (pHierarchy==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pHierarchy->Status&XCFL_HIERARCHY_STATUS_HEADER_READ)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pHierarchy->pLevel==NULL) {
		xcflError Err;

		Err=xcflSource_SetPos(pSource,pHierarchy->LevelOffset);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		Err=xcflLevel_Create(&pHierarchy->pLevel,
							 pHierarchy->BytesPerPixel,pHierarchy->Compression);
		if (Err==XCFL_ERR_SUCCESS) {
			Err=xcflLevel_ReadHeader(pHierarchy->pLevel,pSource);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflLevelInfo Info;

				Err=xcflLevel_GetInfo(pHierarchy->pLevel,&Info);
				if (Err==XCFL_ERR_SUCCESS) {
					if (Info.Width!=pHierarchy->Width
							|| Info.Height!=pHierarchy->Height) {
						Err=XCFL_ERR_BAD_FORMAT;
					} else {
						Err=xcflLevel_ReadData(pHierarchy->pLevel,pSource);
					}
				}
			}

			if (Err!=XCFL_ERR_SUCCESS)
				xcflLevel_Delete(&pHierarchy->pLevel);
		}

		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflHierarchy_IsDataLoaded(const xcflHierarchy *pHierarchy)
{
	if (pHierarchy==NULL || pHierarchy->pLevel==NULL)
		return XCFL_FALSE;

	return xcflLevel_IsDataLoaded(pHierarchy->pLevel);
}


XCFL_EXPORT(void*) xcflHierarchy_GetData(const xcflHierarchy *pHierarchy)
{
	if (pHierarchy==NULL || pHierarchy->pLevel==NULL)
		return NULL;

	return xcflLevel_GetData(pHierarchy->pLevel);
}


XCFL_EXPORT(void*) xcflHierarchy_GetRow(const xcflHierarchy *pHierarchy,xcflUInt Top)
{
	if (pHierarchy==NULL || pHierarchy->pLevel==NULL)
		return NULL;

	return xcflLevel_GetRow(pHierarchy->pLevel,Top);
}
