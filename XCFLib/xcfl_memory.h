/******************************************************************************
*                                                                             *
*    xcfl_memory.h                          Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_MEMORY_H
#define XCFL_MEMORY_H


#ifdef __cplusplus
extern "C" {
#endif


#ifndef XCFL_MEMORY_DEBUG
XCFL_EXPORT(void*) xcflMemoryAlloc(xcflSize Size);
XCFL_EXPORT(void*) xcflMemoryReAlloc(void *pBuffer,xcflSize Size);
XCFL_EXPORT(void) xcflMemoryFree(void *pBuffer);
XCFL_EXPORT(void*) xcflMemoryDuplicate(const void *pBuffer,xcflSize Size);
#else
XCFL_EXPORT(void) xcflMemoryDebugInit(void);
XCFL_EXPORT(void*) xcflMemoryAllocDebug(xcflSize Size,
										const char *pFileName,int Line);
XCFL_EXPORT(void*) xcflMemoryReAllocDebug(void *pBuffer,xcflSize Size,
										  const char *pFileName,int Line);
XCFL_EXPORT(void) xcflMemoryFreeDebug(void *pBuffer);
XCFL_EXPORT(void*) xcflMemoryDuplicateDebug(const void *pBuffer,xcflSize Size,
											const char *pFileName,int Line);
#define xcflMemoryAlloc(size)	xcflMemoryAllocDebug(size,__FILE__,__LINE__)
#define xcflMemoryReAlloc(ptr,size) \
	xcflMemoryReAllocDebug(ptr,size,__FILE__,__LINE__)
#define xcflMemoryFree(ptr)		xcflMemoryFreeDebug(ptr)
#define xcflMemoryDuplicate(ptr,size) \
	xcflMemoryDuplicateDebug(ptr,size,__FILE__,__LINE__)
#endif
XCFL_EXPORT(void) xcflMemoryCopy(void *pDstBuffer,const void *pSrcBuffer,xcflSize Size);
XCFL_EXPORT(void) xcflMemoryMove(void *pDstBuffer,const void *pSrcBuffer,xcflSize Size);
XCFL_EXPORT(void) xcflMemoryFill(void *pBuffer,xcflSize Size,xcflUInt8 Value);
XCFL_EXPORT(int) xcflMemoryCompare(const void *pData1,const void *pData2,xcflSize Size);

#define xcflNew(type) ((type*)xcflMemoryAlloc(sizeof(type)))
#define xcflDelete(ptr) (xcflMemoryFree(ptr),(ptr)=NULL)


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_MEMORY_H */
