/******************************************************************************
*                                                                             *
*    xcfl.c                                 Copyright(c) 2010-2013 itow,y.    *
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




XCFL_EXPORT(xcflUInt32) xcflGetVersion(void)
{
	return XCFL_VERSION;
}


XCFL_EXPORT(const char*) xcflGetVersionString(void)
{
	return XCFL_VERSION_STRING;
}


#ifdef XCFL_WCHAR_SUPPORT
XCFL_EXPORT(const xcflWChar*) xcflGetVersionStringW(void)
{
	return XCFL_WTEXT(XCFL_VERSION_STRING);
}
#endif


XCFL_EXPORT(xcflInt) xcflCheckSignature(const void *pData,xcflSize Size)
{
	const xcflUInt8 *p;

	if (pData==NULL || Size<XCFL_SIGNATURE_BYTES)
		return XCFL_SIGNATURE_INVALID;

	p=(const xcflUInt8*)pData;

	if (xcflMemoryCompare(&p[0],"gimp xcf ",9)!=0)
		return XCFL_SIGNATURE_INVALID;

	if (xcflMemoryCompare(&p[9],"file",5)==0)
		return 0;
	else if (xcflMemoryCompare(&p[9],"v001",5)==0)
		return 1;
	else if (xcflMemoryCompare(&p[9],"v002",5)==0)
		return 2;
	else if (xcflMemoryCompare(&p[9],"v003",5)==0)
		return 3;

	return XCFL_SIGNATURE_UNKNOWN_VERSION;
}


XCFL_EXPORT(xcflUInt) xcflGetImageTypePixelBytes(xcflImageType Type)
{
	switch (Type) {
	case XCFL_IMAGE_TYPE_RGB:				return 3;
	case XCFL_IMAGE_TYPE_RGB_ALPHA:			return 4;
	case XCFL_IMAGE_TYPE_GRAYSCALE:			return 1;
	case XCFL_IMAGE_TYPE_GRAYSCALE_ALPHA:	return 2;
	case XCFL_IMAGE_TYPE_INDEXED:			return 1;
	case XCFL_IMAGE_TYPE_INDEXED_ALPHA:		return 2;
	}

	return 0;
}


XCFL_EXPORT(xcflError) xcflProgress_Init(xcflProgress *pProgress,
										 xcflProgressCallback pCallback,
										 void *pClientData)
{
	if (pProgress==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pProgress->pCallback=pCallback;
	pProgress->pClientData=pClientData;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflProgress_Call(xcflProgress *pProgress,int Pos,int Max)
{
	if (pProgress==NULL || pProgress->pCallback==NULL)
		return XCFL_TRUE;

	return pProgress->pCallback(pProgress,Pos,Max);
}
