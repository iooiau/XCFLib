/******************************************************************************
*                                                                             *
*    xcfl_compress.h                        Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_COMPRESS_H
#define XCFL_COMPRESS_H


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	XCFL_FILE_COMPRESSION_UNKNOWN=-1,
	XCFL_FILE_COMPRESSION_UNCOMPRESSED,
	XCFL_FILE_COMPRESSION_GZIP,
	XCFL_FILE_COMPRESSION_BZIP2
} xcflFileCompression;

#define XCFL_CHECK_FILE_COMPRESSION_BYTES 9
XCFL_EXPORT(xcflFileCompression) xcflCheckFileCompression(const void *pData,xcflSize Size);
XCFL_EXPORT(xcflInt) xcflCheckSourceSignature(xcflSource *pSource,xcflInt *pVersion);
XCFL_EXPORT(xcflError) xcflDecompressSource(xcflSource *pSource,
											xcflDestination *pDest);
#define XCFL_DECOMPRESS_HEADER_PROPERTIES	0x0001
XCFL_EXPORT(xcflError) xcflDecompressHeader(xcflSource *pSource,
											xcflDestination *pDest,
											xcflUInt Flags);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_COMPRESS_H */
