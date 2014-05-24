/******************************************************************************
*                                                                             *
*    xcfl_offset.c                          Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_offset.h"




xcflError xcflOffsetList_Init(xcflOffsetList *pOffsetList)
{
	if (pOffsetList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pOffsetList->pOffsets=NULL;
	pOffsetList->NumOffsets=0;

	return XCFL_ERR_SUCCESS;
}


xcflError xcflOffsetList_Free(xcflOffsetList *pOffsetList)
{
	if (pOffsetList==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pOffsetList->pOffsets!=NULL) {
		xcflMemoryFree(pOffsetList->pOffsets);
		pOffsetList->pOffsets=NULL;
	}
	pOffsetList->NumOffsets=0;

	return XCFL_ERR_SUCCESS;
}


static xcflBool AppendOffsets(xcflOffsetList *pOffsetList,
							  const xcflUInt32 *pOffsets,int NumOffsets)
{
	xcflUInt32 *pNewList;

	pNewList=(xcflUInt32*)xcflMemoryReAlloc(
		pOffsetList->pOffsets,
		(pOffsetList->NumOffsets+NumOffsets)*sizeof(xcflUInt32));
	if (pNewList==NULL) {
		xcflOffsetList_Free(pOffsetList);
		return XCFL_FALSE;
	}

	xcflMemoryCopy(&pNewList[pOffsetList->NumOffsets],
				   pOffsets,NumOffsets*sizeof(xcflUInt32));

	pOffsetList->pOffsets=pNewList;
	pOffsetList->NumOffsets+=NumOffsets;

	return XCFL_TRUE;
}


xcflError xcflOffsetList_Read(xcflOffsetList *pOffsetList,xcflSource *pSource)
{
#define BUFFER_LENGTH 32

	xcflUInt32 Buffer[BUFFER_LENGTH];
	int i,j;

	if (pOffsetList==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflOffsetList_Free(pOffsetList);

	j=0;
	for (i=0;;i++) {
		xcflUInt32 Offset;

		if (!xcflSource_ReadUInt32(pSource,&Offset)) {
			xcflOffsetList_Free(pOffsetList);
			return XCFL_ERR_READ;
		}
		if (Offset==0)
			break;

		Buffer[j++]=Offset;

		if (j==BUFFER_LENGTH) {
			if (!AppendOffsets(pOffsetList,Buffer,BUFFER_LENGTH))
				return XCFL_ERR_MEMORY_ALLOC;
			j=0;
		}
	}

	if (j>0) {
		if (!AppendOffsets(pOffsetList,Buffer,j))
			return XCFL_ERR_MEMORY_ALLOC;
	}

	return XCFL_ERR_SUCCESS;
}
