/******************************************************************************
*                                                                             *
*    xcfl_pixelbuffer.c                     Copyright(c) 2010-2013 itow,y.    *
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


struct xcflPixelBuffer_tag {
	xcflUInt Flags;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt BytesPerPixel;
	xcflInt RowStride;
	xcflUInt8 *pBuffer;
};

#define XCFL_PIXEL_BUFFER_FLAG_ALLOCATED	0x0001




static void *GetRowPtr(const xcflPixelBuffer *pPixelBuffer,xcflUInt Top)
{
	xcflSize Offset;

	if (pPixelBuffer->RowStride<0)
		Offset=(pPixelBuffer->Height-1-Top)*-pPixelBuffer->RowStride;
	else
		Offset=Top*pPixelBuffer->RowStride;
	return pPixelBuffer->pBuffer+Offset;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Create(xcflPixelBuffer **ppPixelBuffer)
{
	xcflPixelBuffer *pPixelBuffer;

	if (ppPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppPixelBuffer=xcflNew(xcflPixelBuffer);
	if (*ppPixelBuffer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pPixelBuffer=*ppPixelBuffer;

	pPixelBuffer->Flags=0;
	pPixelBuffer->Width=0;
	pPixelBuffer->Height=0;
	pPixelBuffer->BytesPerPixel=0;
	pPixelBuffer->RowStride=0;
	pPixelBuffer->pBuffer=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Delete(xcflPixelBuffer **ppPixelBuffer)
{
	if (ppPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppPixelBuffer!=NULL) {
		xcflPixelBuffer *pPixelBuffer=*ppPixelBuffer;

		if ((pPixelBuffer->Flags&XCFL_PIXEL_BUFFER_FLAG_ALLOCATED)!=0)
			xcflMemoryFree(pPixelBuffer->pBuffer);

		xcflDelete(*ppPixelBuffer);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Free(xcflPixelBuffer *pPixelBuffer)
{
	if (pPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer!=NULL) {
		if ((pPixelBuffer->Flags&XCFL_PIXEL_BUFFER_FLAG_ALLOCATED)!=0)
			xcflMemoryFree(pPixelBuffer->pBuffer);
		pPixelBuffer->pBuffer=NULL;
	}

	pPixelBuffer->Flags=0;
	pPixelBuffer->Width=0;
	pPixelBuffer->Height=0;
	pPixelBuffer->BytesPerPixel=0;
	pPixelBuffer->RowStride=0;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Allocate(xcflPixelBuffer *pPixelBuffer,
					xcflUInt32 Width,xcflUInt32 Height,xcflUInt BytesPerPixel)
{
	xcflInt RowBytes;

	if (pPixelBuffer==NULL || Width==0 || Height==0 || BytesPerPixel==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflPixelBuffer_Free(pPixelBuffer);

	RowBytes=xcflRowBytes(Width,BytesPerPixel);
	pPixelBuffer->pBuffer=(xcflUInt8*)xcflMemoryAlloc(RowBytes*Height);
	if (pPixelBuffer->pBuffer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pPixelBuffer->Flags|=XCFL_PIXEL_BUFFER_FLAG_ALLOCATED;
	pPixelBuffer->Width=Width;
	pPixelBuffer->Height=Height;
	pPixelBuffer->BytesPerPixel=BytesPerPixel;
	pPixelBuffer->RowStride=RowBytes;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_SetBuffer(xcflPixelBuffer *pPixelBuffer,
					xcflUInt32 Width,xcflUInt32 Height,xcflUInt BytesPerPixel,
					xcflInt RowStride,void *pBuffer)
{

	if (pPixelBuffer==NULL || Width==0 || Height==0 || BytesPerPixel==0
			|| (xcflUInt)xcflAbs(RowStride)<Width*BytesPerPixel
			|| pBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflPixelBuffer_Free(pPixelBuffer);

	pPixelBuffer->Width=Width;
	pPixelBuffer->Height=Height;
	pPixelBuffer->BytesPerPixel=BytesPerPixel;
	pPixelBuffer->RowStride=RowStride;
	pPixelBuffer->pBuffer=(xcflUInt8*)pBuffer;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_GetInfo(const xcflPixelBuffer *pPixelBuffer,
											   xcflPixelBufferInfo *pInfo)
{
	if (pPixelBuffer==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	pInfo->Width=pPixelBuffer->Width;
	pInfo->Height=pPixelBuffer->Height;
	pInfo->BytesPerPixel=pPixelBuffer->BytesPerPixel;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(void*) xcflPixelBuffer_GetRow(const xcflPixelBuffer *pPixelBuffer,xcflUInt Top)
{
	if (pPixelBuffer==NULL || Top>=pPixelBuffer->Height
			|| pPixelBuffer->pBuffer==NULL)
		return NULL;

	return GetRowPtr(pPixelBuffer,Top);
}


XCFL_EXPORT(void*) xcflPixelBuffer_GetPixelPtr(const xcflPixelBuffer *pPixelBuffer,
											   xcflUInt Left,xcflUInt Top)
{
	if (pPixelBuffer==NULL
			|| Left>=pPixelBuffer->Width
			|| Top>=pPixelBuffer->Height
			|| pPixelBuffer->pBuffer==NULL)
		return NULL;

	return (xcflUInt8*)GetRowPtr(pPixelBuffer,Top)+Left*pPixelBuffer->BytesPerPixel;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_ClearPixels(xcflPixelBuffer *pPixelBuffer)
{
	if (pPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	xcflMemoryFill(pPixelBuffer->pBuffer,
				   xcflAbs(pPixelBuffer->RowStride)*pPixelBuffer->Height,
				   0);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Fill(xcflPixelBuffer *pPixelBuffer,
											const xcflUInt8 *pColor)
{
	xcflUInt Width,Height,BytesPerPixel,x,y;

	if (pPixelBuffer==NULL || pColor==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	Width=pPixelBuffer->Width;
	Height=pPixelBuffer->Height;
	BytesPerPixel=pPixelBuffer->BytesPerPixel;

	for (y=0;y<Height;y++) {
		xcflUInt8 *p;

		p=pPixelBuffer->pBuffer+y*xcflAbs(pPixelBuffer->RowStride);

		for (x=0;x<Width;x++) {
			switch (BytesPerPixel) {
			case 4:	p[3]=pColor[3];
			case 3:	p[2]=pColor[2];
			case 2:	p[1]=pColor[1];
			case 1:	p[0]=pColor[0];
			}
			p+=BytesPerPixel;
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_SwapRGBOrder(xcflPixelBuffer *pPixelBuffer)
{
	xcflUInt Width,Height,x,y;
	xcflUInt BytesPerPixel;
	xcflUInt8 Temp;

	if (pPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer==NULL || pPixelBuffer->BytesPerPixel<3)
		return XCFL_ERR_UNEXPECTED_CALL;

	Width=pPixelBuffer->Width;
	Height=pPixelBuffer->Height;
	BytesPerPixel=pPixelBuffer->BytesPerPixel;

	for (y=0;y<Height;y++) {
		xcflUInt8 *p;

		p=pPixelBuffer->pBuffer+y*xcflAbs(pPixelBuffer->RowStride);

		for (x=0;x<Width;x++) {
			Temp=p[0];
			p[0]=p[2];
			p[2]=Temp;
			p+=BytesPerPixel;
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflPixelBuffer_Matte(xcflPixelBuffer *pPixelBuffer,
											 const xcflUInt8 *pBackColor)
{
	xcflUInt Width,Height,x,y;
	xcflUInt BytesPerPixel,AlphaOffset,i;

	if (pPixelBuffer==NULL || pBackColor==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pPixelBuffer->pBuffer==NULL || pPixelBuffer->BytesPerPixel<2)
		return XCFL_ERR_UNEXPECTED_CALL;

	Width=pPixelBuffer->Width;
	Height=pPixelBuffer->Height;
	BytesPerPixel=pPixelBuffer->BytesPerPixel;
	AlphaOffset=BytesPerPixel-1;

	for (y=0;y<Height;y++) {
		xcflUInt8 *p;

		p=pPixelBuffer->pBuffer+y*xcflAbs(pPixelBuffer->RowStride);

		for (x=0;x<Width;x++) {
			xcflUInt Alpha,Beta;

			Alpha=p[AlphaOffset];
			if (Alpha!=255) {
				Beta=255-Alpha;
				for (i=0;i<AlphaOffset;i++)
					p[i]=xcflDivideBy255(p[i]*Alpha+pBackColor[i]*Beta);
				p[AlphaOffset]=255;
			}
			p+=BytesPerPixel;
		}
	}

	return XCFL_ERR_SUCCESS;
}
