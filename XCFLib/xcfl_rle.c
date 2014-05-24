/******************************************************************************
*                                                                             *
*    xcfl_rle.c                             Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_rle.h"




xcflError xcflRLEDecode(xcflBufferedSource *pSource,
						xcflUInt32 Width,xcflUInt32 Height,xcflUInt32 BytesPerPixel,
						void *pDstData)
{
#define READ_BYTE(v) \
	if (SrcBuffer.Remain==0) {									\
		Err=xcflBufferedSource_Read(pSource,&SrcBuffer,256);	\
		if (Err!=XCFL_ERR_SUCCESS)								\
			return Err;											\
	}															\
	(v)=*SrcBuffer.pBuffer++;									\
	SrcBuffer.Remain--

	xcflSourceBuffer SrcBuffer;
	xcflError Err;
	xcflUInt i;

	if (pSource==NULL || Width==0 || Height==0 || BytesPerPixel==0
			|| pDstData==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	SrcBuffer.Remain=0;

	for (i=0;i<BytesPerPixel;i++) {
		xcflSize PixelSize;
		xcflUInt8 *q;

		PixelSize=Width*Height;
		q=(xcflUInt8*)pDstData+i;
		while (PixelSize>0) {
			xcflUInt Length;
			xcflUInt8 Value;

			READ_BYTE(Length);
			if (Length>=128) {
				if (Length>128) {
					Length=256-Length;
				} else {
					READ_BYTE(Length);
					READ_BYTE(Value);
					Length=(Length<<8)|Value;
				}
				if (Length>PixelSize)
					Length=PixelSize;
				PixelSize-=Length;
				while (Length-->0) {
					READ_BYTE(*q);
					q+=BytesPerPixel;
				}
			} else {
				if (Length<127) {
					Length++;
				} else {
					READ_BYTE(Length);
					READ_BYTE(Value);
					Length=(Length<<8)|Value;
				}
				if (Length>PixelSize)
					Length=PixelSize;
				PixelSize-=Length;
				READ_BYTE(Value);
				while (Length-->0) {
					*q=Value;
					q+=BytesPerPixel;
				}
			}
		}
	}

	return XCFL_ERR_SUCCESS;
}
