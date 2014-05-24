/******************************************************************************
*                                                                             *
*    xcfl_level.c                           Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_level.h"
#include "xcfl_offset.h"
#include "xcfl_tile.h"


struct xcflLevel_tag {
	xcflUInt Status;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt32 BytesPerPixel;
	xcflCompression Compression;
	xcflOffsetList TileOffsets;
	void *pData;
};

#define XCFL_LEVEL_STATUS_HEADER_READ	0x0001




XCFL_EXPORT(xcflError) xcflLevel_Create(xcflLevel **ppLevel,
										xcflUInt32 BytesPerPixel,
										xcflCompression Compression)
{
	xcflLevel *pLevel;

	if (ppLevel==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (BytesPerPixel==0) {
		*ppLevel=NULL;
		return XCFL_ERR_INVALID_ARGUMENT;
	}

	*ppLevel=xcflNew(xcflLevel);
	if (*ppLevel==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pLevel=*ppLevel;

	pLevel->Status=0;
	pLevel->Width=0;
	pLevel->Height=0;
	pLevel->BytesPerPixel=BytesPerPixel;
	pLevel->Compression=Compression;
	xcflOffsetList_Init(&pLevel->TileOffsets);
	pLevel->pData=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLevel_Delete(xcflLevel **ppLevel)
{
	if (ppLevel==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppLevel!=NULL) {
		xcflLevel *pLevel=*ppLevel;

		xcflOffsetList_Free(&pLevel->TileOffsets);
		xcflMemoryFree(pLevel->pData);

		xcflDelete(*ppLevel);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLevel_ReadHeader(xcflLevel *pLevel,xcflSource *pSource)
{
	xcflUInt32 Width,Height;
	xcflError Err;

	if (pLevel==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLevel->Status&XCFL_LEVEL_STATUS_HEADER_READ)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (!xcflSource_ReadUInt32(pSource,&Width)
			|| !xcflSource_ReadUInt32(pSource,&Height))
		return XCFL_ERR_READ;

	Err=xcflOffsetList_Read(&pLevel->TileOffsets,pSource);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if (pLevel->TileOffsets.NumOffsets==0)
		return XCFL_ERR_BAD_FORMAT;

	pLevel->Width=Width;
	pLevel->Height=Height;
	pLevel->Status|=XCFL_LEVEL_STATUS_HEADER_READ;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLevel_GetInfo(const xcflLevel *pLevel,xcflLevelInfo *pInfo)
{
	if (pLevel==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLevel->Status&XCFL_LEVEL_STATUS_HEADER_READ)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	pInfo->Width=pLevel->Width;
	pInfo->Height=pLevel->Height;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLevel_ReadData(xcflLevel *pLevel,xcflSource *pSource)
{
	if (pLevel==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLevel->Status&XCFL_LEVEL_STATUS_HEADER_READ)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pLevel->pData==NULL) {
		xcflError Err;

		pLevel->pData=xcflMemoryAlloc(
			xcflRowBytes(pLevel->Width,pLevel->BytesPerPixel)*pLevel->Height);
		if (pLevel->pData==NULL)
			return XCFL_ERR_MEMORY_ALLOC;

		Err=xcflReadTiles(pSource,&pLevel->TileOffsets,
						  pLevel->Width,pLevel->Height,pLevel->BytesPerPixel,
						  pLevel->Compression,pLevel->pData);
		if (Err!=XCFL_ERR_SUCCESS) {
			xcflMemoryFree(pLevel->pData);
			pLevel->pData=NULL;
			return Err;
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflLevel_IsDataLoaded(const xcflLevel *pLevel)
{
	if (pLevel==NULL)
		return XCFL_FALSE;

	return pLevel->pData!=NULL;
}


XCFL_EXPORT(void*) xcflLevel_GetData(const xcflLevel *pLevel)
{
	if (pLevel==NULL)
		return NULL;

	return pLevel->pData;
}


XCFL_EXPORT(void*) xcflLevel_GetRow(const xcflLevel *pLevel,xcflUInt Top)
{
	if (pLevel==NULL || pLevel->pData==NULL || Top>=pLevel->Height)
		return NULL;

	return (xcflUInt8*)pLevel->pData+
						Top*xcflRowBytes(pLevel->Width,pLevel->BytesPerPixel);
}
