/******************************************************************************
*                                                                             *
*    xcfl_compress.c                        Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_decompress.h"
#include "xcfl_util.h"




XCFL_EXPORT(xcflFileCompression) xcflCheckFileCompression(const void *pData,xcflSize Size)
{
	const xcflUInt8 *p;

	if (pData==NULL || Size<XCFL_CHECK_FILE_COMPRESSION_BYTES)
		return XCFL_FILE_COMPRESSION_UNKNOWN;

	p=(const xcflUInt8*)pData;

	if (xcflMemoryCompare(p,"gimp xcf ",9)==0)
		return XCFL_FILE_COMPRESSION_UNCOMPRESSED;

	if (p[0]==0x1F && p[1]==0x8B)
		return XCFL_FILE_COMPRESSION_GZIP;

	if (p[0]==0x42 && p[1]==0x5A && p[2]==0x68)	/* "BZh" */
		return XCFL_FILE_COMPRESSION_BZIP2;

	return XCFL_FILE_COMPRESSION_UNKNOWN;
}


XCFL_EXPORT(xcflError) xcflCheckSourceSignature(xcflSource *pSource,xcflInt *pVersion)
{
	xcflUInt8 Magic[XCFL_CHECK_FILE_COMPRESSION_BYTES];
	xcflFileCompression Compression;
	xcflError Err;
	xcflDecompress *pDecompress;

	if (pVersion!=NULL)
		*pVersion=XCFL_SIGNATURE_INVALID;

	if (pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (xcflSource_Read(pSource,Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES)!=
											XCFL_CHECK_FILE_COMPRESSION_BYTES)
		return XCFL_ERR_READ;

	xcflSource_SetPos(pSource,0);

	Compression=xcflCheckFileCompression(Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES);
	if (Compression==XCFL_FILE_COMPRESSION_UNKNOWN)
		return XCFL_ERR_INVALID_FORMAT;

	Err=xcflDecompress_Create(&pDecompress,pSource,Compression);
	if (Err==XCFL_ERR_SUCCESS) {
		Err=xcflDecompress_ReadHeader(pDecompress);
		if (Err==XCFL_ERR_SUCCESS) {
			xcflUInt8 Buffer[XCFL_SIGNATURE_BYTES];
			xcflSize Size;

			Size=XCFL_SIGNATURE_BYTES;
			Err=xcflDecompress_Decompress(pDecompress,Buffer,&Size);
			if (Err==XCFL_ERR_SUCCESS) {
				if (Size==XCFL_SIGNATURE_BYTES) {
					xcflInt Version=xcflCheckSignature(Buffer,XCFL_SIGNATURE_BYTES);

					if (pVersion!=NULL)
						*pVersion=Version;
					else if (Version==XCFL_SIGNATURE_INVALID)
						Err=XCFL_ERR_INVALID_FORMAT;
					else if (Version==XCFL_SIGNATURE_UNKNOWN_VERSION)
						Err=XCFL_ERR_UNSUPPORTED_FORMAT;
				} else {
					Err=XCFL_ERR_INVALID_FORMAT;
				}
			}
		}
		xcflDecompress_Delete(&pDecompress);
	}

	return Err;
}


XCFL_EXPORT(xcflError) xcflDecompressSource(xcflSource *pSource,
											xcflDestination *pDest)
{
#define DECOMPRESS_BUFFER_SIZE (64*1024)

	xcflUInt8 Magic[XCFL_CHECK_FILE_COMPRESSION_BYTES];
	xcflFileCompression Compression;
	xcflError Err;
	xcflDecompress *pDecompress;
	void *pBuffer;

	if (pSource==NULL || pDest==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (xcflSource_Read(pSource,Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES)!=
											XCFL_CHECK_FILE_COMPRESSION_BYTES)
		return XCFL_ERR_READ;

	xcflSource_SetPos(pSource,0);

	Compression=xcflCheckFileCompression(Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES);
	if (Compression==XCFL_FILE_COMPRESSION_UNKNOWN)
		return XCFL_ERR_INVALID_FORMAT;

	pBuffer=xcflMemoryAlloc(DECOMPRESS_BUFFER_SIZE);
	if (pBuffer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	Err=xcflDecompress_Create(&pDecompress,pSource,Compression);
	if (Err==XCFL_ERR_SUCCESS) {
		Err=xcflDecompress_ReadHeader(pDecompress);
		if (Err==XCFL_ERR_SUCCESS) {
			xcflSize Size;

			do {
				Size=DECOMPRESS_BUFFER_SIZE;
				Err=xcflDecompress_Decompress(pDecompress,pBuffer,&Size);
				if (Err!=XCFL_ERR_SUCCESS)
					break;
				if (Size>0) {
					if (xcflDestination_Write(pDest,pBuffer,Size)!=Size) {
						Err=XCFL_ERR_WRITE;
						break;
					}
				}
			} while (Size==DECOMPRESS_BUFFER_SIZE);
		}
		xcflDecompress_Delete(&pDecompress);
	}

	xcflMemoryFree(pBuffer);

	return Err;
}


static xcflError ReadDecompress(xcflDecompress *pDecompress,
								void *pBuffer,xcflSize Size)
{
	xcflSize ReadedSize;
	xcflError Err;

	ReadedSize=Size;
	Err=xcflDecompress_Decompress(pDecompress,pBuffer,&ReadedSize);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;
	if (ReadedSize!=Size)
		return XCFL_ERR_READ;
	return XCFL_ERR_SUCCESS;
}

static xcflError DecompressProperties(xcflDecompress *pDecompress,
									  xcflDestination *pDest,
									  void *pBuffer,xcflSize BufferSize)
{
	while (XCFL_TRUE) {
		xcflError Err;
		xcflUInt8 Buffer[8];
		xcflUInt32 Type,Size;

		Err=ReadDecompress(pDecompress,Buffer,8);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
		Type=xcflGetMSBFirst32(Buffer+0);
		Size=xcflGetMSBFirst32(Buffer+4);
		if (xcflDestination_Write(pDest,Buffer,8)!=8)
			return XCFL_ERR_WRITE;

		if (Type==XCFL_PROPERTY_END)
			break;

		if (Type==XCFL_PROPERTY_COLORMAP) {
			xcflUInt32 NumColors;

			if (Size<4)
				return XCFL_ERR_BAD_FORMAT;

			Err=ReadDecompress(pDecompress,Buffer,4);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;

			NumColors=xcflGetMSBFirst32(Buffer);
			if (Size==4+NumColors)
				Size=4+3*NumColors;
			else if (4+NumColors*3>Size)
				return XCFL_ERR_BAD_FORMAT;

			if (xcflDestination_Write(pDest,Buffer,4)!=4)
				return XCFL_ERR_WRITE;
			Size-=4;
		}

		if (Size>0) {
			do {
				xcflSize ReadSize=xcflMin(Size,BufferSize);

				Err=ReadDecompress(pDecompress,pBuffer,ReadSize);
				if (Err!=XCFL_ERR_SUCCESS)
					return Err;
				if (xcflDestination_Write(pDest,pBuffer,ReadSize)!=ReadSize)
					return XCFL_ERR_WRITE;
				Size-=ReadSize;
			} while (Size>0);
		}
	}

	return XCFL_ERR_SUCCESS;
}

static xcflError DecompressOffsets(xcflDecompress *pDecompress,
								   xcflDestination *pDest)
{
	while (XCFL_TRUE) {
		xcflError Err;
		xcflUInt8 Buffer[4];

		Err=ReadDecompress(pDecompress,Buffer,4);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		if (xcflDestination_Write(pDest,Buffer,4)!=4)
			return XCFL_ERR_WRITE;

		if (xcflGetMSBFirst32(Buffer)==0)
			break;
	}

	return XCFL_ERR_SUCCESS;
}

XCFL_EXPORT(xcflError) xcflDecompressHeader(xcflSource *pSource,
											xcflDestination *pDest,
											xcflUInt Flags)
{
	xcflUInt8 Magic[XCFL_CHECK_FILE_COMPRESSION_BYTES];
	xcflFileCompression Compression;
	xcflError Err;
	xcflDecompress *pDecompress;
	xcflUInt8 *pBuffer;

	if (pSource==NULL || pDest==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (xcflSource_Read(pSource,Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES)!=
											XCFL_CHECK_FILE_COMPRESSION_BYTES)
		return XCFL_ERR_READ;

	Compression=xcflCheckFileCompression(Magic,XCFL_CHECK_FILE_COMPRESSION_BYTES);
	if (Compression==XCFL_FILE_COMPRESSION_UNKNOWN)
		return XCFL_ERR_INVALID_FORMAT;

	pBuffer=(xcflUInt8*)xcflMemoryAlloc(DECOMPRESS_BUFFER_SIZE);
	if (pBuffer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	xcflSource_SetPos(pSource,0);

	Err=xcflDecompress_Create(&pDecompress,pSource,Compression);
	if (Err==XCFL_ERR_SUCCESS) {
		Err=xcflDecompress_ReadHeader(pDecompress);
		if (Err==XCFL_ERR_SUCCESS) {
			Err=ReadDecompress(pDecompress,pBuffer,XCFL_SIGNATURE_BYTES);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflInt Version=xcflCheckSignature(pBuffer,XCFL_SIGNATURE_BYTES);
				if (Version<0) {
					if (Version==XCFL_SIGNATURE_INVALID)
						Err=XCFL_ERR_INVALID_FORMAT;
					else
						Err=XCFL_ERR_UNSUPPORTED_FORMAT;
				} else if (xcflDestination_Write(pDest,pBuffer,
								XCFL_SIGNATURE_BYTES)!=XCFL_SIGNATURE_BYTES) {
					Err=XCFL_ERR_WRITE;
				} else {
					Err=ReadDecompress(pDecompress,pBuffer,12);
					if (Err==XCFL_ERR_SUCCESS) {
						xcflUInt32 Width,Height,BaseType;

						Width=xcflGetMSBFirst32(pBuffer+0);
						Height=xcflGetMSBFirst32(pBuffer+4);
						BaseType=xcflGetMSBFirst32(pBuffer+8);
						if (Width==0 || Height==0) {
							Err=XCFL_ERR_BAD_FORMAT;
						} else if (BaseType>XCFL_IMAGE_BASE_TYPE_LAST) {
							Err=XCFL_ERR_UNSUPPORTED_FORMAT;
						} else if (xcflDestination_Write(pDest,pBuffer,12)!=12) {
							Err=XCFL_ERR_WRITE;
						} else if ((Flags&XCFL_DECOMPRESS_HEADER_PROPERTIES)!=0) {
							Err=DecompressProperties(pDecompress,pDest,
											pBuffer,DECOMPRESS_BUFFER_SIZE);
							if (Err==XCFL_ERR_SUCCESS) {
								/* Layer offsets */
								Err=DecompressOffsets(pDecompress,pDest);
								if (Err==XCFL_ERR_SUCCESS) {
									/* Channel offsets */
									Err=DecompressOffsets(pDecompress,pDest);
								}
							}
						}
					}
				}
			}
		}
		xcflDecompress_Delete(&pDecompress);
	}

	xcflMemoryFree(pBuffer);

	return Err;
}
