/******************************************************************************
*                                                                             *
*    xcfl_memory.c                          Copyright(c) 2010-2013 itow,y.    *
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




#ifndef XCFL_MEMORY_DEBUG


XCFL_EXPORT(void*) xcflMemoryAlloc(xcflSize Size)
{
	if (Size==0)
		return NULL;

	return malloc(Size);
}


XCFL_EXPORT(void*) xcflMemoryReAlloc(void *pBuffer,xcflSize Size)
{
	if (pBuffer==NULL && Size==0)
		return NULL;

	return realloc(pBuffer,Size);
}


XCFL_EXPORT(void) xcflMemoryFree(void *pBuffer)
{
	free(pBuffer);
}


XCFL_EXPORT(void*) xcflMemoryDuplicate(const void *pBuffer,xcflSize Size)
{
	void *pNewBuffer;

	if (pBuffer==NULL || Size==0)
		return NULL;

	pNewBuffer=malloc(Size);
	if (pNewBuffer==NULL)
		return NULL;

	memcpy(pNewBuffer,pBuffer,Size);

	return pNewBuffer;
}


#else	/* ndef XCFL_MEMORY_DEBUG */


#include <crtdbg.h>


XCFL_EXPORT(void) xcflMemoryDebugInit(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF/* | _CRTDBG_CHECK_ALWAYS_DF*/);
}


XCFL_EXPORT(void*) xcflMemoryAllocDebug(xcflSize Size,
										const char *pFileName,int Line)
{
	if (Size==0)
		return NULL;

	return _malloc_dbg(Size,_NORMAL_BLOCK,pFileName,Line);
}


XCFL_EXPORT(void*) xcflMemoryReAllocDebug(void *pBuffer,xcflSize Size,
										  const char *pFileName,int Line)
{
	if (pBuffer==NULL && Size==0)
		return NULL;

	return _realloc_dbg(pBuffer,Size,_NORMAL_BLOCK,pFileName,Line);
}


XCFL_EXPORT(void) xcflMemoryFreeDebug(void *pBuffer)
{
	_free_dbg(pBuffer,_NORMAL_BLOCK);
}


XCFL_EXPORT(void*) xcflMemoryDuplicateDebug(const void *pBuffer,xcflSize Size,
											const char *pFileName,int Line)
{
	void *pNewBuffer;

	if (pBuffer==NULL || Size==0)
		return NULL;

	pNewBuffer=_malloc_dbg(Size,_NORMAL_BLOCK,pFileName,Line);
	if (pNewBuffer==NULL)
		return NULL;

	memcpy(pNewBuffer,pBuffer,Size);

	return pNewBuffer;
}


#endif	/* XCFL_MEMORY_DEBUG */


XCFL_EXPORT(void) xcflMemoryCopy(void *pDstBuffer,const void *pSrcBuffer,xcflSize Size)
{
	if (pDstBuffer==NULL || pSrcBuffer==NULL || Size==0)
		return;

	memcpy(pDstBuffer,pSrcBuffer,Size);
}


XCFL_EXPORT(void) xcflMemoryMove(void *pDstBuffer,const void *pSrcBuffer,xcflSize Size)
{
	if (pDstBuffer==NULL || pSrcBuffer==NULL || Size==0)
		return;

	memmove(pDstBuffer,pSrcBuffer,Size);
}


XCFL_EXPORT(void) xcflMemoryFill(void *pBuffer,xcflSize Size,xcflUInt8 Value)
{
	if (pBuffer==NULL || Size==0)
		return;

	memset(pBuffer,Value,Size);
}


XCFL_EXPORT(int) xcflMemoryCompare(const void *pData1,const void *pData2,xcflSize Size)
{
	if (pData1==NULL || pData2==NULL || Size==0)
		return 0;

	return memcmp(pData1,pData2,Size);
}
