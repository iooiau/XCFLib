/******************************************************************************
*                                                                             *
*    xcfl_item.c                            Copyright(c) 2010-2013 itow,y.    *
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




xcflError xcflItemPath_Init(xcflItemPath *pPath)
{
	if (pPath==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pPath->NumElements=0;
	pPath->pList=NULL;

	return XCFL_ERR_SUCCESS;
}


xcflError xcflItemPath_Free(xcflItemPath *pPath)
{
	if (pPath==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pPath->NumElements=0;
	if (pPath->pList!=NULL) {
		xcflMemoryFree(pPath->pList);
		pPath->pList=NULL;
	}

	return XCFL_ERR_SUCCESS;
}


xcflError xcflItemPath_Allocate(xcflItemPath *pPath,xcflUInt NumElements)
{
	if (pPath==NULL || NumElements==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPath->NumElements<NumElements) {
		xcflItemPath_Free(pPath);
		pPath->pList=(xcflUInt32*)xcflMemoryAlloc(NumElements*sizeof(xcflUInt32));
		if (pPath->pList==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
	}
	pPath->NumElements=NumElements;

	return XCFL_ERR_SUCCESS;
}
