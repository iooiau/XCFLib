/******************************************************************************
*                                                                             *
*    xcfl_stdsource.c                       Copyright(c) 2010-2013 itow,y.    *
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


#if defined(_MSC_VER) && _MSC_VER>=1400
#define XCFL_FOPEN_S_SUPPORT
#endif




static XCFL_CALLBACK_DECL(xcflSize,FileSourceRead)(void *pClientData,
												   void *pBuffer,xcflSize Size)
{
	return fread(pBuffer,1,Size,(FILE*)pClientData);
}


static XCFL_CALLBACK_DECL(xcflError,FileSourceSetPos)(void *pClientData,
													  xcflSize Pos)
{
	if (Pos>LONG_MAX)
		return XCFL_ERR_SET_POSITION;

	if (fseek((FILE*)pClientData,Pos,SEEK_SET)!=0)
		return XCFL_ERR_SET_POSITION;

	return XCFL_ERR_SUCCESS;
}


static XCFL_CALLBACK_DECL(xcflError,FileSourceClose)(void *pClientData)
{
	fclose((FILE*)pClientData);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflSource_CreateFileSource(xcflSource **ppSource,
												   FILE *pFile,xcflBool Close)
{
	if (ppSource==NULL || pFile==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	return xcflSource_Create(ppSource,pFile,
							 FileSourceRead,FileSourceSetPos,
							 Close?FileSourceClose:NULL);
}


XCFL_EXPORT(xcflError) xcflSource_OpenFile(xcflSource **ppSource,
										   const char *pFileName,xcflUInt Flags)
{
	const char *pMode;
	FILE *pFile;
	xcflError Err;

	if (ppSource==NULL || pFileName==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	switch (Flags&(XCFL_FILE_READ | XCFL_FILE_WRITE)) {
	case XCFL_FILE_READ:	pMode="rb";	break;
	case XCFL_FILE_WRITE:	pMode="wb";	break;
	default:
		return XCFL_ERR_INVALID_ARGUMENT;
	}

#ifndef XCFL_FOPEN_S_SUPPORT
	pFile=fopen(pFileName,pMode);
	if (pFile==NULL)
		return XCFL_ERR_FILE_OPEN;
#else
	if (fopen_s(&pFile,pFileName,pMode)!=0)
		return XCFL_ERR_FILE_OPEN;
#endif

	Err=xcflSource_Create(ppSource,pFile,
						  FileSourceRead,FileSourceSetPos,FileSourceClose);
	if (Err!=XCFL_ERR_SUCCESS) {
		fclose(pFile);
		return Err;
	}

	return XCFL_ERR_SUCCESS;
}


#if defined(XCFL_WCHAR_SUPPORT) && defined(XCFL_WFOPEN_SUPPORT)

XCFL_EXPORT(xcflError) xcflSource_OpenFileW(xcflSource **ppSource,
											const xcflWChar *pFileName,xcflUInt Flags)
{
	const xcflWChar *pMode;
	FILE *pFile;
	xcflError Err;

	if (ppSource==NULL || pFileName==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	switch (Flags&(XCFL_FILE_READ | XCFL_FILE_WRITE)) {
	case XCFL_FILE_READ:	pMode=XCFL_WTEXT("rb");	break;
	case XCFL_FILE_WRITE:	pMode=XCFL_WTEXT("wb");	break;
	default:
		return XCFL_ERR_INVALID_ARGUMENT;
	}

#ifndef XCFL_FOPEN_S_SUPPORT
	pFile=_wfopen(pFileName,pMode);
	if (pFile==NULL)
		return XCFL_ERR_FILE_OPEN;
#else
	if (_wfopen_s(&pFile,pFileName,pMode)!=0)
		return XCFL_ERR_FILE_OPEN;
#endif

	Err=xcflSource_Create(ppSource,pFile,
						  FileSourceRead,FileSourceSetPos,FileSourceClose);
	if (Err!=XCFL_ERR_SUCCESS) {
		fclose(pFile);
		return Err;
	}

	return XCFL_ERR_SUCCESS;
}

#endif




typedef struct {
	const xcflUInt8 *pData;
	xcflSize Size;
	xcflSize Pos;
} xcflMemorySource;


static XCFL_CALLBACK_DECL(xcflSize,MemorySourceRead)(void *pClientData,
													 void *pBuffer,xcflSize Size)
{
	xcflMemorySource *pMemorySource=(xcflMemorySource*)pClientData;

	if (pMemorySource->Pos>=pMemorySource->Size)
		return 0;

	if (Size>pMemorySource->Size-pMemorySource->Pos)
		Size=pMemorySource->Size-pMemorySource->Pos;

	xcflMemoryCopy(pBuffer,pMemorySource->pData+pMemorySource->Pos,Size);
	pMemorySource->Pos+=Size;

	return Size;
}


static XCFL_CALLBACK_DECL(xcflError,MemorySourceSetPos)(void *pClientData,
														xcflSize Pos)
{
	xcflMemorySource *pMemorySource=(xcflMemorySource*)pClientData;

	if (Pos>pMemorySource->Size)
		return XCFL_ERR_SET_POSITION;

	pMemorySource->Pos=Pos;

	return XCFL_ERR_SUCCESS;
}


static XCFL_CALLBACK_DECL(xcflError,MemorySourceClose)(void *pClientData)
{
	xcflMemoryFree(pClientData);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflSource_CreateMemorySource(xcflSource **ppSource,
													 const void *pData,xcflSize Size)
{
	xcflMemorySource *pMemorySource;
	xcflError Err;

	if (ppSource==NULL || pData==NULL || Size==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	pMemorySource=xcflNew(xcflMemorySource);
	if (pMemorySource==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pMemorySource->pData=(const xcflUInt8*)pData;
	pMemorySource->Size=Size;
	pMemorySource->Pos=0;

	Err=xcflSource_Create(ppSource,pMemorySource,
						  MemorySourceRead,MemorySourceSetPos,MemorySourceClose);
	if (Err!=XCFL_ERR_SUCCESS)
		xcflMemoryFree(pMemorySource);

	return Err;
}




#ifdef XCFL_WINDOWS


#include <windows.h>


static XCFL_CALLBACK_DECL(xcflSize,HandleSourceRead)(void *pClientData,
													 void *pBuffer,xcflSize Size)
{
	DWORD Read;

	if (!ReadFile((HANDLE)pClientData,pBuffer,Size,&Read,NULL))
		return 0;

	return Read;
}


static XCFL_CALLBACK_DECL(xcflError,HandleSourceSetPos)(void *pClientData,
														xcflSize Pos)
{
	LARGE_INTEGER li;

	li.QuadPart=Pos;
	li.LowPart=SetFilePointer((HANDLE)pClientData,li.LowPart,&li.HighPart,FILE_BEGIN);
	if ((li.LowPart==INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)
			|| li.QuadPart!=(LONGLONG)Pos)
		return XCFL_ERR_SET_POSITION;

	return XCFL_ERR_SUCCESS;
}


static XCFL_CALLBACK_DECL(xcflError,HandleSourceClose)(void *pClientData)
{
	CloseHandle((HANDLE)pClientData);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflSource_CreateHandleSource(xcflSource **ppSource,
													 void *Handle,xcflBool Close)
{
	if (ppSource==NULL || Handle==INVALID_HANDLE_VALUE)
		return XCFL_ERR_INVALID_ARGUMENT;

	return xcflSource_Create(ppSource,Handle,
							 HandleSourceRead,HandleSourceSetPos,
							 Close?HandleSourceClose:NULL);
}


#endif	/* XCFL_WINDOWS */
