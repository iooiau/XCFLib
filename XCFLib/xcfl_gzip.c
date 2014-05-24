/******************************************************************************
*                                                                             *
*    xcfl_gzip.c                            Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_gzip.h"
#include "zlib/zlib.h"


struct xcflGZip_tag {
	xcflSource *pSource;
	z_stream ZStream;
	void *pBuffer;
	xcflSize BufferSize;
	xcflUInt Status;
};

#define XCFL_GZIP_STATUS_HEADER_READ			0x0001
#define XCFL_GZIP_STATUS_ZSTREAM_INITIALIZED	0x0002

#define XCFL_GZIP_BUFFER_SIZE	(64*1024)

#define XCFL_GZIP_FLAG_ASCII		0x01
#define XCFL_GZIP_FLAG_CONTINUATION	0x02
#define XCFL_GZIP_FLAG_EXTRA_FIELD	0x04
#define XCFL_GZIP_FLAG_ORIG_NAME	0x08
#define XCFL_GZIP_FLAG_COMMENT		0x10
#define XCFL_GZIP_FLAG_ENCRYPTED	0x20
#define XCFL_GZIP_FLAG_RESERVED		0xC0

#define XCFL_GZIP_METHOD_DEFLATE	8




static xcflError ZLibResultToXCFLError(int Result)
{
	switch (Result) {
	case Z_OK:				return XCFL_ERR_SUCCESS;
	case Z_STREAM_ERROR:	return XCFL_ERR_INTERNAL;
	case Z_DATA_ERROR:		return XCFL_ERR_BAD_FORMAT;
	case Z_MEM_ERROR:		return XCFL_ERR_MEMORY_ALLOC;
	}

	return XCFL_ERR_INTERNAL;
}


XCFL_EXPORT(xcflError) xcflGZip_Create(xcflGZip **ppGZip,xcflSource *pSource)
{
	xcflGZip *pGZip;

	if (ppGZip==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppGZip=xcflNew(xcflGZip);
	if (*ppGZip==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pGZip=*ppGZip;

	pGZip->pSource=pSource;
	pGZip->pBuffer=NULL;
	pGZip->BufferSize=0;
	pGZip->Status=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflGZip_Delete(xcflGZip **ppGZip)
{
	if (ppGZip==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppGZip!=NULL) {
		xcflGZip *pGZip=*ppGZip;

		if ((pGZip->Status&XCFL_GZIP_STATUS_ZSTREAM_INITIALIZED)!=0)
			inflateEnd(&pGZip->ZStream);
		xcflMemoryFree(pGZip->pBuffer);

		xcflDelete(*ppGZip);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflGZip_ReadHeader(xcflGZip *pGZip)
{
	xcflUInt8 Buffer[10];
	xcflUInt8 Flags;

	if (pGZip==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pGZip->Status&XCFL_GZIP_STATUS_HEADER_READ)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (xcflSource_Read(pGZip->pSource,Buffer,10)!=10)
		return XCFL_ERR_READ;

	if (Buffer[0]!=0x1F || Buffer[1]!=0x8B)
		return XCFL_ERR_INVALID_FORMAT;

	if (Buffer[2]!=XCFL_GZIP_METHOD_DEFLATE)
		return XCFL_ERR_UNSUPPORTED_FORMAT;

	Flags=Buffer[3];
	if ((Flags&XCFL_GZIP_FLAG_ENCRYPTED)!=0
			|| (Flags&XCFL_GZIP_FLAG_CONTINUATION)!=0)
		return XCFL_ERR_UNSUPPORTED_FORMAT;

	/* Buffer[4..7] Time stamp */
	/* Buffer[8]    Ext flag */
	/* Buffer[9]    OS type */

	/*
	if ((Flags&XCFL_GZIP_FLAG_CONTINUATION)!=0) {
		if (xcflSource_Read(pGZip->pSource,Buffer,1)!=1)
			return XCFL_ERR_READ;
		Part=Buffer[0];
	}
	*/
	/*
	if ((Flags&XCFL_GZIP_FLAG_EXTRA_FIELD)!=0) {
		if (xcflSource_Read(pGZip->pSource,Buffer,1)!=1)
			return XCFL_ERR_READ;
		Length=Buffer[0];
	}
	if ((Flags&XCFL_GZIP_FLAG_ORIG_NAME)!=0) {
	}
	if ((Flags&XCFL_GZIP_FLAG_COMMENT)!=0) {
	}
	*/

	xcflSource_SetPos(pGZip->pSource,0);

	pGZip->Status|=XCFL_GZIP_STATUS_HEADER_READ;

	return XCFL_ERR_SUCCESS;
}


#ifdef XCFL_MEMORY_DEBUG
static voidpf GZipAlloc(voidpf opaque,uInt items,uInt size)
{
	return xcflMemoryAlloc(items*size);
}

static void GZipFree(voidpf opaque,voidpf address)
{
	xcflMemoryFree(address);
}
#endif

static xcflSize ReadSource(xcflGZip *pGZip)
{
	xcflSize ReadedSize;

	ReadedSize=xcflSource_Read(pGZip->pSource,pGZip->pBuffer,pGZip->BufferSize);
	if (ReadedSize==0)
		return 0;

	pGZip->ZStream.next_in=(Bytef*)pGZip->pBuffer;
	pGZip->ZStream.avail_in=ReadedSize;

	return ReadedSize;
}

XCFL_EXPORT(xcflError) xcflGZip_Decompress(xcflGZip *pGZip,
										   void *pBuffer,xcflSize *pSize)
{
	int Result;
	xcflSize ReadedSize;

	if (pGZip==NULL || pBuffer==NULL || pSize==NULL || *pSize==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pGZip->Status&XCFL_GZIP_STATUS_HEADER_READ)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pGZip->pBuffer==NULL) {
		pGZip->pBuffer=xcflMemoryAlloc(XCFL_GZIP_BUFFER_SIZE);
		if (pGZip->pBuffer==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
		pGZip->BufferSize=XCFL_GZIP_BUFFER_SIZE;
	}

	if ((pGZip->Status&XCFL_GZIP_STATUS_ZSTREAM_INITIALIZED)==0) {
#ifndef XCFL_MEMORY_DEBUG
		pGZip->ZStream.zalloc=NULL;
		pGZip->ZStream.zfree=NULL;
#else
		pGZip->ZStream.zalloc=GZipAlloc;
		pGZip->ZStream.zfree=GZipFree;
#endif
		pGZip->ZStream.opaque=NULL;

		pGZip->ZStream.avail_in=0;
		pGZip->ZStream.next_in=NULL;

		Result=inflateInit2(&pGZip->ZStream,0x1F);
		if (Result!=Z_OK)
			return ZLibResultToXCFLError(Result);

		pGZip->Status|=XCFL_GZIP_STATUS_ZSTREAM_INITIALIZED;
	}

	pGZip->ZStream.next_out=pBuffer;
	pGZip->ZStream.avail_out=(uInt)*pSize;

	while (pGZip->ZStream.avail_out>0) {
		if (pGZip->ZStream.avail_in==0) {
			ReadedSize=ReadSource(pGZip);
			if (ReadedSize==0)
				break;
		}

		Result=inflate(&pGZip->ZStream,Z_NO_FLUSH);
		if (Result==Z_STREAM_END)
			break;
		if (Result!=Z_OK)
			return ZLibResultToXCFLError(Result);
	}

	*pSize-=pGZip->ZStream.avail_out;

	return XCFL_ERR_SUCCESS;
}
