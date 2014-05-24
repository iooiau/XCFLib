/******************************************************************************
*                                                                             *
*    xcfl_bzip2.c                           Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_bzip2.h"
#include "bzip2/bzlib.h"


struct xcflBZip2_tag {
	xcflSource *pSource;
	bz_stream BZStream;
	void *pBuffer;
	xcflSize BufferSize;
	xcflUInt Status;
};

#define XCFL_BZIP2_STATUS_HEADER_READED			0x0001
#define XCFL_BZIP2_STATUS_BZSTREAM_INITIALIZED	0x0002

#define XCFL_BZIP2_BUFFER_SIZE	(64*1024)




#ifdef BZ_NO_STDIO
#if defined(XCFL_WINDOWS) && defined(XCFL_DEBUG)
#include <windows.h>
void bz_internal_error(int errcode)
{
	char szText[64];
	wsprintfA(szText,"bz_internal_error(%d)\n",errcode);
	OutputDebugStringA(szText);
}
#else
void bz_internal_error(int errcode)
{
}
#endif
#endif	/* ndef BZ_NO_STDIO */


static xcflError LibBZ2ResultToXCFLError(int Result)
{
	switch (Result) {
	case BZ_OK:					return XCFL_ERR_SUCCESS;
	case BZ_PARAM_ERROR:		return XCFL_ERR_INTERNAL;
	case BZ_MEM_ERROR:			return XCFL_ERR_MEMORY_ALLOC;
	case BZ_DATA_ERROR:			return XCFL_ERR_BAD_FORMAT;
	case BZ_DATA_ERROR_MAGIC:	return XCFL_ERR_INVALID_FORMAT;
	case BZ_UNEXPECTED_EOF:		return XCFL_ERR_BAD_FORMAT;
	case BZ_CONFIG_ERROR:		return XCFL_ERR_INTERNAL;
	}

	return XCFL_ERR_INTERNAL;
}


XCFL_EXPORT(xcflError) xcflBZip2_Create(xcflBZip2 **ppBZip2,xcflSource *pSource)
{
	xcflBZip2 *pBZip2;

	if (ppBZip2==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppBZip2=xcflNew(xcflBZip2);
	if (*ppBZip2==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pBZip2=*ppBZip2;

	pBZip2->pSource=pSource;
	pBZip2->pBuffer=NULL;
	pBZip2->BufferSize=0;
	pBZip2->Status=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflBZip2_Delete(xcflBZip2 **ppBZip2)
{
	if (ppBZip2==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppBZip2!=NULL) {
		xcflBZip2 *pBZip2=*ppBZip2;

		if ((pBZip2->Status&XCFL_BZIP2_STATUS_BZSTREAM_INITIALIZED)!=0)
			BZ2_bzDecompressEnd(&pBZip2->BZStream);
		xcflMemoryFree(pBZip2->pBuffer);

		xcflDelete(*ppBZip2);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflBZip2_ReadHeader(xcflBZip2 *pBZip2)
{
	xcflUInt8 Buffer[3];

	if (pBZip2==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pBZip2->Status&XCFL_BZIP2_STATUS_HEADER_READED)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (xcflSource_Read(pBZip2->pSource,Buffer,3)!=3)
		return XCFL_ERR_READ;

	if (Buffer[0]!=0x42 || Buffer[1]!=0x5A || Buffer[2]!=0x68)	/* "BZh" */
		return XCFL_ERR_INVALID_FORMAT;

	xcflSource_SetPos(pBZip2->pSource,0);

	pBZip2->Status|=XCFL_BZIP2_STATUS_HEADER_READED;

	return XCFL_ERR_SUCCESS;
}


#ifdef XCFL_MEMORY_DEBUG
static void *BZip2Alloc(void *opaque,int items,int size)
{
	return xcflMemoryAlloc(items*size);
}

static void BZip2Free(void *opaque,void *address)
{
	xcflMemoryFree(address);
}
#endif

static xcflSize ReadSource(xcflBZip2 *pBZip2)
{
	xcflSize ReadedSize;

	ReadedSize=xcflSource_Read(pBZip2->pSource,
							   pBZip2->pBuffer,pBZip2->BufferSize);
	if (ReadedSize==0)
		return 0;

	pBZip2->BZStream.next_in=(char*)pBZip2->pBuffer;
	pBZip2->BZStream.avail_in=ReadedSize;

	return ReadedSize;
}

XCFL_EXPORT(xcflError) xcflBZip2_Decompress(xcflBZip2 *pBZip2,
											void *pBuffer,xcflSize *pSize)
{
	int Result;
	xcflSize ReadedSize;

	if (pBZip2==NULL || pBuffer==NULL || pSize==NULL || *pSize==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pBZip2->Status&XCFL_BZIP2_STATUS_HEADER_READED)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pBZip2->pBuffer==NULL) {
		pBZip2->pBuffer=xcflMemoryAlloc(XCFL_BZIP2_BUFFER_SIZE);
		if (pBZip2->pBuffer==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
		pBZip2->BufferSize=XCFL_BZIP2_BUFFER_SIZE;
	}

	if ((pBZip2->Status&XCFL_BZIP2_STATUS_BZSTREAM_INITIALIZED)==0) {
#ifndef XCFL_MEMORY_DEBUG
		pBZip2->BZStream.bzalloc=NULL;
		pBZip2->BZStream.bzfree=NULL;
#else
		pBZip2->BZStream.bzalloc=BZip2Alloc;
		pBZip2->BZStream.bzfree=BZip2Free;
#endif
		pBZip2->BZStream.opaque=NULL;

		pBZip2->BZStream.next_in=NULL;
		pBZip2->BZStream.avail_in=0;

		Result=BZ2_bzDecompressInit(&pBZip2->BZStream,0,0);
		if (Result!=BZ_OK)
			return LibBZ2ResultToXCFLError(Result);

		pBZip2->Status|=XCFL_BZIP2_STATUS_BZSTREAM_INITIALIZED;
	}

	pBZip2->BZStream.next_out=pBuffer;
	pBZip2->BZStream.avail_out=(unsigned int)*pSize;

	while (pBZip2->BZStream.avail_out>0) {
		if (pBZip2->BZStream.avail_in==0) {
			ReadedSize=ReadSource(pBZip2);
			if (ReadedSize==0)
				break;
		}

		Result=BZ2_bzDecompress(&pBZip2->BZStream);
		if (Result==BZ_STREAM_END)
			break;
		if (Result!=BZ_OK)
			return LibBZ2ResultToXCFLError(Result);
	}

	*pSize-=pBZip2->BZStream.avail_out;

	return XCFL_ERR_SUCCESS;
}
