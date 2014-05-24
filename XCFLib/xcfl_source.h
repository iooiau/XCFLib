/******************************************************************************
*                                                                             *
*    xcfl_source.h                          Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_SOURCE_H
#define XCFL_SOURCE_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflSource_tag xcflSource;
typedef XCFL_CALLBACK_TYPE(xcflSize,xcflSource_ReadFunc)(void *pClientData,
										void *pBuffer,xcflSize Size);
typedef XCFL_CALLBACK_TYPE(xcflError,xcflSource_SetPosFunc)(void *pClientData,xcflSize Pos);
typedef XCFL_CALLBACK_TYPE(xcflError,xcflSource_CloseFunc)(void *pClientData);

typedef struct xcflBufferedSource_tag xcflBufferedSource;

typedef struct {
	const xcflUInt8 *pBuffer;
	xcflSize Remain;
} xcflSourceBuffer;


XCFL_EXPORT(xcflError) xcflSource_Create(xcflSource **ppSource,
	void *pClientData,xcflSource_ReadFunc Read,xcflSource_SetPosFunc SetPos,
	xcflSource_CloseFunc Close);
XCFL_EXPORT(xcflError) xcflSource_Delete(xcflSource **ppSource);
XCFL_EXPORT(xcflSize) xcflSource_Read(xcflSource *pSource,
									  void *pBuffer,xcflSize Size);
XCFL_EXPORT(xcflError) xcflSource_SetPos(xcflSource *pSource,xcflSize Pos);
XCFL_EXPORT(xcflBool) xcflSource_ReadUInt32(xcflSource *pSource,xcflUInt32 *pData);
XCFL_EXPORT(void*) xcflSource_GetClientData(const xcflSource *pSource);

XCFL_EXPORT(xcflError) xcflBufferedSource_Create(xcflBufferedSource **ppBufferedSource,
												 xcflSource *pSource,
												 xcflSize BufferSize);
XCFL_EXPORT(xcflError) xcflBufferedSource_Delete(xcflBufferedSource **ppBufferedSource);
XCFL_EXPORT(xcflError) xcflBufferedSource_Reset(xcflBufferedSource *pBufferedSource);
XCFL_EXPORT(xcflError) xcflBufferedSource_Read(xcflBufferedSource *pBufferedSource,
											   xcflSourceBuffer *pSourceBuffer,
											   xcflSize Size);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_SOURCE_H */
