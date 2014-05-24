/******************************************************************************
*                                                                             *
*    xcfl_stdsource.h                       Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_STD_SOURCE_H
#define XCFL_STD_SOURCE_H


#ifdef __cplusplus
extern "C" {
#endif


#define XCFL_FILE_READ	0x0001
#define XCFL_FILE_WRITE	0x0002


XCFL_EXPORT(xcflError) xcflSource_CreateFileSource(xcflSource **ppSource,
												   FILE *pFile,xcflBool Close);
XCFL_EXPORT(xcflError) xcflSource_OpenFile(xcflSource **ppSource,
										   const char *pFileName,xcflUInt Flags);
#if defined(XCFL_WCHAR_SUPPORT) && defined(XCFL_WFOPEN_SUPPORT)
XCFL_EXPORT(xcflError) xcflSource_OpenFileW(xcflSource **ppSource,
											const xcflWChar *pFileName,xcflUInt Flags);
#endif
XCFL_EXPORT(xcflError) xcflSource_CreateMemorySource(xcflSource **ppSource,
													 const void *pData,xcflSize Size);
#ifdef XCFL_WINDOWS
XCFL_EXPORT(xcflError) xcflSource_CreateHandleSource(xcflSource **ppSource,
													 void *Handle,xcflBool Close);
#endif


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_STD_SOURCE_H */
