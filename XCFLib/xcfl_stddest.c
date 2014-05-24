/******************************************************************************
*                                                                             *
*    xcfl_stddest.c                         Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_stddest.h"


struct xcflMemoryDest_tag {
	xcflDestination *pDest;
	xcflUInt8 *pBuffer;
	xcflSize BufferSize;
	xcflSize WrittenSize;
};




static XCFL_CALLBACK_DECL(xcflSize,MemoryDestWrite)(void *pClientData,
													const void *pBuffer,xcflSize Size)
{
	xcflMemoryDest *pMemoryDest=(xcflMemoryDest*)pClientData;

	if (Size>pMemoryDest->BufferSize-pMemoryDest->WrittenSize) {
		xcflSize AllocUnit,NewSize;
		xcflUInt8 *pNewBuffer;

		if (pMemoryDest->BufferSize<8*1024*1024) {
			AllocUnit=1024*1024;
		} else {
			AllocUnit=(pMemoryDest->BufferSize/(4*1024*1024))*(1024*1024);
		}
		NewSize=(pMemoryDest->WrittenSize+Size+AllocUnit)/AllocUnit*AllocUnit;
		pNewBuffer=(xcflUInt8*)xcflMemoryReAlloc(pMemoryDest->pBuffer,NewSize);
		if (pNewBuffer==NULL)
			return 0;

		pMemoryDest->pBuffer=pNewBuffer;
		pMemoryDest->BufferSize=NewSize;
	}

	xcflMemoryCopy(pMemoryDest->pBuffer+pMemoryDest->WrittenSize,
				   pBuffer,Size);
	pMemoryDest->WrittenSize+=Size;

	return Size;
}


XCFL_EXPORT(xcflError) xcflMemoryDest_Create(xcflMemoryDest **ppMemoryDest)
{
	xcflMemoryDest *pMemoryDest;
	xcflError Err;

	if (ppMemoryDest==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppMemoryDest=xcflNew(xcflMemoryDest);
	if (*ppMemoryDest==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pMemoryDest=*ppMemoryDest;

	Err=xcflDestination_Create(&pMemoryDest->pDest,
							   pMemoryDest,MemoryDestWrite,NULL);
	if (Err!=XCFL_ERR_SUCCESS) {
		xcflDelete(*ppMemoryDest);
		return Err;
	}

	pMemoryDest->pBuffer=NULL;
	pMemoryDest->BufferSize=0;
	pMemoryDest->WrittenSize=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflMemoryDest_Delete(xcflMemoryDest **ppMemoryDest)
{
	if (ppMemoryDest==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppMemoryDest!=NULL) {
		xcflMemoryDest *pMemoryDest=*ppMemoryDest;

		xcflDestination_Delete(&pMemoryDest->pDest);
		xcflMemoryFree(pMemoryDest->pBuffer);

		xcflDelete(*ppMemoryDest);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflDestination*) xcflMemoryDest_GetDestination(const xcflMemoryDest *pMemoryDest)
{
	if (pMemoryDest==NULL)
		return NULL;

	return pMemoryDest->pDest;
}


XCFL_EXPORT(xcflError) xcflMemoryDest_CreateSource(const xcflMemoryDest *pMemoryDest,
												   xcflSource **ppSource)
{
	if (pMemoryDest==NULL || ppSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pMemoryDest->pBuffer==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	return xcflSource_CreateMemorySource(ppSource,
										 pMemoryDest->pBuffer,
										 pMemoryDest->WrittenSize);
}


XCFL_EXPORT(xcflError) xcflMemoryDest_GetBuffer(const xcflMemoryDest *pMemoryDest,
												xcflMemoryDestBuffer *pBuffer)
{
	if (pMemoryDest==NULL || pBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pMemoryDest->pBuffer==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	pBuffer->pData=pMemoryDest->pBuffer;
	pBuffer->Size=pMemoryDest->WrittenSize;

	return XCFL_ERR_SUCCESS;
}
