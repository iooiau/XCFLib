/******************************************************************************
*                                                                             *
*    xcfl_decompress.c                      Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_compress.h"
#include "xcfl_decompress.h"
#include "xcfl_gzip.h"
#include "xcfl_bzip2.h"


struct xcflDecompress_tag {
	xcflSource *pSource;
	xcflFileCompression Compression;
	union {
		xcflGZip *pGZip;
		xcflBZip2 *pBZip2;
	} Data;
};




XCFL_EXPORT(xcflError) xcflDecompress_Create(xcflDecompress **ppDecompress,
											 xcflSource *pSource,
											 xcflFileCompression Compression)
{
	xcflDecompress *pDecompress;
	xcflError Err;

	if (ppDecompress==NULL || pSource==NULL
			|| (Compression!=XCFL_FILE_COMPRESSION_UNCOMPRESSED
				&& Compression!=XCFL_FILE_COMPRESSION_GZIP
				&& Compression!=XCFL_FILE_COMPRESSION_BZIP2))
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppDecompress=xcflNew(xcflDecompress);
	if (*ppDecompress==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pDecompress=*ppDecompress;

	pDecompress->pSource=pSource;
	pDecompress->Compression=Compression;

	if (Compression!=XCFL_FILE_COMPRESSION_UNCOMPRESSED) {
		switch (Compression) {
		case XCFL_FILE_COMPRESSION_GZIP:
			Err=xcflGZip_Create(&pDecompress->Data.pGZip,pSource);
			break;

		case XCFL_FILE_COMPRESSION_BZIP2:
			Err=xcflBZip2_Create(&pDecompress->Data.pBZip2,pSource);
			break;
		}

		if (Err!=XCFL_ERR_SUCCESS) {
			xcflDelete(*ppDecompress);
			return Err;
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflDecompress_Delete(xcflDecompress **ppDecompress)
{
	if (ppDecompress==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppDecompress!=NULL) {
		xcflDecompress *pDecompress=*ppDecompress;

		switch (pDecompress->Compression) {
		case XCFL_FILE_COMPRESSION_GZIP:
			xcflGZip_Delete(&pDecompress->Data.pGZip);
			break;

		case XCFL_FILE_COMPRESSION_BZIP2:
			xcflBZip2_Delete(&pDecompress->Data.pBZip2);
			break;
		}

		xcflDelete(*ppDecompress);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflDecompress_ReadHeader(xcflDecompress *pDecompress)
{
	xcflError Err;

	if (pDecompress==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	switch (pDecompress->Compression) {
	case XCFL_FILE_COMPRESSION_GZIP:
		Err=xcflGZip_ReadHeader(pDecompress->Data.pGZip);
		break;

	case XCFL_FILE_COMPRESSION_BZIP2:
		Err=xcflBZip2_ReadHeader(pDecompress->Data.pBZip2);
		break;

	default:
		Err=XCFL_ERR_SUCCESS;
	}

	return Err;
}


XCFL_EXPORT(xcflError) xcflDecompress_Decompress(xcflDecompress *pDecompress,
												 void *pBuffer,xcflSize *pSize)
{
	xcflError Err;

	if (pDecompress==NULL || pBuffer==NULL || pSize==NULL || *pSize==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	Err=XCFL_ERR_SUCCESS;

	switch (pDecompress->Compression) {
	case XCFL_FILE_COMPRESSION_UNCOMPRESSED:
		*pSize=xcflSource_Read(pDecompress->pSource,pBuffer,*pSize);
		if (*pSize==0)
			Err=XCFL_ERR_READ;
		break;

	case XCFL_FILE_COMPRESSION_GZIP:
		Err=xcflGZip_Decompress(pDecompress->Data.pGZip,pBuffer,pSize);
		break;

	case XCFL_FILE_COMPRESSION_BZIP2:
		Err=xcflBZip2_Decompress(pDecompress->Data.pBZip2,pBuffer,pSize);
		break;

	default:
		return XCFL_ERR_INTERNAL;
	}

	return Err;
}
