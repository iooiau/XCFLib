/******************************************************************************
*                                                                             *
*    xcfl_tile.c                            Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_tile.h"
#include "xcfl_rle.h"


#define XCFL_TILE_WIDTH		64
#define XCFL_TILE_HEIGHT	64




xcflError xcflReadTile(xcflSource *pSource,
					   xcflUInt32 Width,xcflUInt32 Height,xcflUInt32 BytesPerPixel,
					   xcflCompression Compression,
					   void *pDstBuffer)
{
	if (pSource==NULL
			|| Width==0 || Height==0 || BytesPerPixel==0
			|| pDstBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	switch (Compression) {
	case XCFL_COMPRESSION_UNCOMPRESSED:
		{
			xcflSize Size=Width*BytesPerPixel*Height;

			if (xcflSource_Read(pSource,pDstBuffer,Size)!=Size)
				return XCFL_ERR_READ;
		}
		break;

	case XCFL_COMPRESSION_RLE:
		{
			xcflError Err;
			xcflBufferedSource *pBufferedSource;

			Err=xcflBufferedSource_Create(&pBufferedSource,pSource,0);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
			Err=xcflRLEDecode(pBufferedSource,
							  Width,Height,BytesPerPixel,pDstBuffer);
			xcflBufferedSource_Delete(&pBufferedSource);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
		}
		break;

	default:
		return XCFL_ERR_INVALID_ARGUMENT;
	}

	return XCFL_ERR_SUCCESS;
}


static void CopyTile(xcflUInt32 Width,xcflUInt32 Height,xcflUInt32 BytesPerPixel,
					 void *pDstData,xcflUInt32 Left,xcflUInt32 Top,
					 xcflUInt32 TileWidth,xcflUInt32 TileHeight,const void *pTileData)
{
	xcflSize SrcRowBytes,DstRowBytes;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt32 y;

	SrcRowBytes=TileWidth*BytesPerPixel;
	DstRowBytes=xcflRowBytes(Width,BytesPerPixel);

	p=(const xcflUInt8*)pTileData;
	q=(xcflUInt8*)pDstData+Top*DstRowBytes+Left*BytesPerPixel;

	for (y=0;y<TileHeight;y++) {
		xcflMemoryCopy(q,p,SrcRowBytes);
		p+=SrcRowBytes;
		q+=DstRowBytes;
	}
}


xcflError xcflReadTiles(xcflSource *pSource,
						const xcflOffsetList *pOffsetList,
						xcflUInt32 Width,xcflUInt32 Height,xcflUInt32 BytesPerPixel,
						xcflCompression Compression,
						void *pDstBuffer)
{
	xcflUInt TileColumns,TileRows,x,y;
	void *pTileBuffer;
	xcflError Err;

	if (pSource==NULL || pOffsetList==NULL
			|| Width==0 || Height==0 || BytesPerPixel==0
			|| (Compression!=XCFL_COMPRESSION_UNCOMPRESSED
				&& Compression!=XCFL_COMPRESSION_RLE)
			|| pDstBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	TileColumns=(Width+(XCFL_TILE_WIDTH-1))/XCFL_TILE_WIDTH;
	TileRows=(Height+(XCFL_TILE_HEIGHT-1))/XCFL_TILE_HEIGHT;

	if (pOffsetList->NumOffsets<TileColumns*TileRows)
		return XCFL_ERR_BAD_FORMAT;

	pTileBuffer=xcflMemoryAlloc(xcflMin(XCFL_TILE_WIDTH,Width)*BytesPerPixel*
								xcflMin(XCFL_TILE_HEIGHT,Height));
	if (pTileBuffer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	for (y=0;y<TileRows;y++) {
		xcflUInt32 TileHeight=xcflMin(XCFL_TILE_HEIGHT,Height-y*XCFL_TILE_HEIGHT);

		for (x=0;x<TileColumns;x++) {
			xcflUInt32 TileWidth=xcflMin(XCFL_TILE_WIDTH,Width-x*XCFL_TILE_WIDTH);

			Err=xcflSource_SetPos(pSource,pOffsetList->pOffsets[y*TileColumns+x]);
			if (Err!=XCFL_ERR_SUCCESS) {
				xcflMemoryFree(pTileBuffer);
				return Err;
			}

			Err=xcflReadTile(pSource,TileWidth,TileHeight,BytesPerPixel,
							 Compression,pTileBuffer);
			if (Err!=XCFL_ERR_SUCCESS) {
				xcflMemoryFree(pTileBuffer);
				return Err;
			}

			CopyTile(Width,Height,BytesPerPixel,pDstBuffer,
					 x*XCFL_TILE_WIDTH,y*XCFL_TILE_HEIGHT,
					 TileWidth,TileHeight,pTileBuffer);
		}
	}

	xcflMemoryFree(pTileBuffer);

	return XCFL_ERR_SUCCESS;
}
