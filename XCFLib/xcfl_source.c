/******************************************************************************
*                                                                             *
*    xcfl_source.c                          Copyright(c) 2010-2013 itow,y.    *
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


struct xcflSource_tag {
	void *pClientData;
	xcflSource_ReadFunc Read;
	xcflSource_SetPosFunc SetPos;
	xcflSource_CloseFunc Close;
};

struct xcflBufferedSource_tag {
	xcflSource *pSource;
	xcflSize BufferSize;
	xcflUInt8 *pBuffer;
	xcflSize BufferPos;
	xcflSize BufferRemain;
};




XCFL_EXPORT(xcflError) xcflSource_Create(xcflSource **ppSource,
	void *pClientData,xcflSource_ReadFunc Read,xcflSource_SetPosFunc SetPos,
	xcflSource_CloseFunc Close)
{
	xcflSource *pSource;

	if (ppSource==NULL || Read==NULL || SetPos==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppSource=xcflNew(xcflSource);
	if (*ppSource==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pSource=*ppSource;

	pSource->pClientData=pClientData;
	pSource->Read=Read;
	pSource->SetPos=SetPos;
	pSource->Close=Close;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflSource_Delete(xcflSource **ppSource)
{
	if (ppSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppSource!=NULL) {
		xcflSource *pSource=*ppSource;

		if (pSource->Close!=NULL)
			(*pSource->Close)(pSource->pClientData);

		xcflDelete(*ppSource);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflSize) xcflSource_Read(xcflSource *pSource,
									  void *pBuffer,xcflSize Size)
{
	if (pSource==NULL || pSource->Read==NULL || pBuffer==NULL || Size==0)
		return 0;
	return (*pSource->Read)(pSource->pClientData,pBuffer,Size);
}


XCFL_EXPORT(xcflError) xcflSource_SetPos(xcflSource *pSource,xcflSize Pos)
{
	if (pSource==NULL || pSource->SetPos==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	return (*pSource->SetPos)(pSource->pClientData,Pos);
}


XCFL_EXPORT(xcflBool) xcflSource_ReadUInt32(xcflSource *pSource,xcflUInt32 *pData)
{
	xcflUInt8 Buffer[4];

	if (pSource==NULL || pData==NULL)
		return XCFL_FALSE;
	if (xcflSource_Read(pSource,Buffer,4)!=4)
		return XCFL_FALSE;
	*pData=((xcflUInt32)Buffer[0]<<24) | ((xcflUInt32)Buffer[1]<<16) |
			((xcflUInt32)Buffer[2]<<8) | (xcflUInt32)Buffer[3];
	return XCFL_TRUE;
}


XCFL_EXPORT(void*) xcflSource_GetClientData(const xcflSource *pSource)
{
	if (pSource==NULL)
		return NULL;

	return pSource->pClientData;
}




XCFL_EXPORT(xcflError) xcflBufferedSource_Create(xcflBufferedSource **ppBufferedSource,
												 xcflSource *pSource,
												 xcflSize BufferSize)
{
	xcflBufferedSource *pBufferedSource;

	if (ppBufferedSource==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppBufferedSource=xcflNew(xcflBufferedSource);
	if (*ppBufferedSource==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pBufferedSource=*ppBufferedSource;

	pBufferedSource->pSource=pSource;
	pBufferedSource->BufferSize=BufferSize;
	if (BufferSize>0) {
		pBufferedSource->pBuffer=(xcflUInt8*)xcflMemoryAlloc(BufferSize);
		if (pBufferedSource->pBuffer==NULL) {
			xcflDelete(*ppBufferedSource);
			return XCFL_ERR_MEMORY_ALLOC;
		}
	} else {
		pBufferedSource->pBuffer=NULL;
	}
	pBufferedSource->BufferPos=0;
	pBufferedSource->BufferRemain=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflBufferedSource_Delete(xcflBufferedSource **ppBufferedSource)
{
	if (ppBufferedSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppBufferedSource!=NULL) {
		xcflMemoryFree((*ppBufferedSource)->pBuffer);
		xcflDelete(*ppBufferedSource);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflBufferedSource_Reset(xcflBufferedSource *pBufferedSource)
{
	if (pBufferedSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pBufferedSource->BufferPos=0;
	pBufferedSource->BufferRemain=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflBufferedSource_Read(xcflBufferedSource *pBufferedSource,
											   xcflSourceBuffer *pSourceBuffer,
											   xcflSize Size)
{
	if (pBufferedSource==NULL || pSourceBuffer==NULL || Size==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pBufferedSource->BufferSize<Size) {
		xcflUInt8 *pNewBuffer;

		pNewBuffer=(xcflUInt8*)xcflMemoryAlloc(Size);
		if (pNewBuffer==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
		pBufferedSource->BufferSize=Size;
		if (pBufferedSource->BufferRemain>0) {
			xcflMemoryCopy(pNewBuffer,
						   pBufferedSource->pBuffer+pBufferedSource->BufferPos,
						   pBufferedSource->BufferRemain);
		}
		xcflMemoryFree(pBufferedSource->pBuffer);
		pBufferedSource->pBuffer=pNewBuffer;
		pBufferedSource->BufferPos=0;
	}

	if (Size<=pBufferedSource->BufferRemain) {
		pBufferedSource->BufferRemain-=Size;
	} else {
		xcflSize Offset,ReadedSize;

		if (Size>pBufferedSource->BufferSize-pBufferedSource->BufferPos) {
			if (pBufferedSource->BufferRemain>0) {
				xcflMemoryMove(pBufferedSource->pBuffer,
							   pBufferedSource->pBuffer+pBufferedSource->BufferPos,
							   pBufferedSource->BufferRemain);
			}
			pBufferedSource->BufferPos=0;
		}

		Offset=pBufferedSource->BufferPos+pBufferedSource->BufferRemain;
		ReadedSize=xcflSource_Read(pBufferedSource->pSource,
								   pBufferedSource->pBuffer+Offset,
								   /*pBufferedSource->BufferSize-Offset*/
								   Size-pBufferedSource->BufferRemain);
		if (ReadedSize==0)
			return XCFL_ERR_READ;
		if (Size<pBufferedSource->BufferRemain+ReadedSize) {
			pBufferedSource->BufferRemain=
				pBufferedSource->BufferRemain+ReadedSize-Size;
		} else if (Size>pBufferedSource->BufferRemain+ReadedSize) {
			Size=pBufferedSource->BufferRemain+ReadedSize;
			pBufferedSource->BufferRemain=0;
		}
	}

	pSourceBuffer->pBuffer=pBufferedSource->pBuffer+pBufferedSource->BufferPos;
	pSourceBuffer->Remain=Size;

	pBufferedSource->BufferPos+=Size;

	return XCFL_ERR_SUCCESS;
}
