/******************************************************************************
*                                                                             *
*    xcfl_image.h                           Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_IMAGE_H
#define XCFL_IMAGE_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflImage_tag xcflImage;

typedef struct {
	xcflInt Version;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflImageBaseType BaseType;
} xcflImageHeader;

typedef struct {
	xcflCompression Compression;
	xcflFloat32 XResolution;
	xcflFloat32 YResolution;
	xcflResolutionUnit ResolutionUnit;
} xcflImageInfo;


XCFL_EXPORT(xcflError) xcflImage_Create(xcflImage **ppImage,xcflSource *pSource);
XCFL_EXPORT(xcflError) xcflImage_Delete(xcflImage **ppImage);
XCFL_EXPORT(xcflError) xcflImage_ReadHeader(xcflImage *pImage);
XCFL_EXPORT(xcflError) xcflImage_GetHeader(xcflImage *pImage,xcflImageHeader *pHeader);
XCFL_EXPORT(xcflError) xcflImage_ReadProperties(xcflImage *pImage);
XCFL_EXPORT(xcflBool) xcflImage_GetProperty(const xcflImage *pImage,xcflProperty *pProperty);
XCFL_EXPORT(xcflError) xcflImage_GetInfo(const xcflImage *pImage,
										 xcflImageInfo *pInfo);
XCFL_EXPORT(xcflError) xcflImage_GetCompression(const xcflImage *pImage,
												xcflCompression *pCompression);
XCFL_EXPORT(xcflError) xcflImage_GetColormap(const xcflImage *pImage,
											 xcflRGB24 *pColormap,xcflUInt *pNumColors);
XCFL_EXPORT(xcflInt) xcflImage_GetNumLayers(const xcflImage *pImage);
XCFL_EXPORT(xcflError) xcflImage_ReadLayer(xcflImage *pImage,
										   int Index,xcflLayer **ppLayer);
XCFL_EXPORT(xcflError) xcflImage_ReadLayers(xcflImage *pImage);
XCFL_EXPORT(xcflLayer*) xcflImage_GetTopLayer(const xcflImage *pImage);
XCFL_EXPORT(xcflLayer*) xcflImage_GetBottomLayer(const xcflImage *pImage);
XCFL_EXPORT(xcflInt) xcflImage_GetNumChannels(const xcflImage *pImage);
XCFL_EXPORT(xcflError) xcflImage_ReadChannel(xcflImage *pImage,
											 int Index,xcflChannel **ppChannel);
XCFL_EXPORT(xcflError) xcflImage_GetCompositedPixels(xcflImage *pImage,
						xcflPixelBuffer *pPixelBuffer,xcflProgress *pProgress);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_IMAGE_H */
