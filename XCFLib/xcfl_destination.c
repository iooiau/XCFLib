/******************************************************************************
*                                                                             *
*    xcfl_destination.c                     Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_destination.h"


struct xcflDestination_tag {
	void *pClientData;
	xcflDestination_WriteFunc Write;
	xcflDestination_CloseFunc Close;
};




XCFL_EXPORT(xcflError) xcflDestination_Create(xcflDestination **ppDest,
	void *pClientData,xcflDestination_WriteFunc Write,xcflDestination_CloseFunc Close)
{
	xcflDestination *pDest;

	if (ppDest==NULL || Write==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppDest=xcflNew(xcflDestination);
	if (*ppDest==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pDest=*ppDest;

	pDest->pClientData=pClientData;
	pDest->Write=Write;
	pDest->Close=Close;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflDestination_Delete(xcflDestination **ppDest)
{
	if (ppDest==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppDest!=NULL) {
		xcflDestination *pDest=*ppDest;

		if (pDest->Close!=NULL)
			pDest->Close(pDest->pClientData);

		xcflDelete(*ppDest);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflSize) xcflDestination_Write(xcflDestination *pDest,
											const void *pBuffer,xcflSize Size)
{
	if (pDest==NULL || pBuffer==NULL || Size==0)
		return 0;

	return pDest->Write(pDest->pClientData,pBuffer,Size);
}


XCFL_EXPORT(void*) xcflDestination_GetClientData(const xcflDestination *pDest)
{
	if (pDest==NULL)
		return NULL;

	return pDest->pClientData;
}
