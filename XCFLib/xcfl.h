/******************************************************************************
*                                                                             *
*    xcfl.h                                 Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_H
#define XCFL_H


#include "xcfl_config.h"


#define XCFL_VERSION_(major,minor,release) \
	(((major)<<24) | ((minor)<<12) | (release))
#define XCFL_VERSION		XCFL_VERSION_(0,3,1)
#define XCFL_VERSION_STRING	"0.3.1"

#define xcflRowBytes(width,bytes_per_pixel) ((width)*(bytes_per_pixel))

#if defined(_MSC_VER)
#pragma intrinsic(abs)
#define xcflAbs	abs
#else
#define xcflAbs(a)				((a)<0?-(a):(a))
#endif
#define xcflMin(a,b)			((a)<(b)?(a):(b))
#define xcflMax(a,b)			((a)>(b)?(a):(b))
#define xcflClamp(a,min,max)	((a)<(min)?(min):(a)>(max)?(max):(a))
#define xcflClamp8(a)			xcflClamp(a,0,255)
/* #define xcflDivideBy255(a)		((a)/255) */
/* Must be 0 <= a <= 255*255 */
#define xcflDivideBy255(a)		((((xcflUInt)(a)+1)*257)>>16)


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	XCFL_ERR_SUCCESS,
	XCFL_ERR_INVALID_ARGUMENT,
	XCFL_ERR_UNEXPECTED_CALL,
	XCFL_ERR_INTERNAL,
	XCFL_ERR_USER_ABORT,
	XCFL_ERR_MEMORY_ALLOC,
	XCFL_ERR_MEMORY_LOCK,
	XCFL_ERR_FILE_OPEN,
	XCFL_ERR_READ,
	XCFL_ERR_WRITE,
	XCFL_ERR_SET_POSITION,
	XCFL_ERR_INVALID_FORMAT,
	XCFL_ERR_BAD_FORMAT,
	XCFL_ERR_UNSUPPORTED_FORMAT,
	XCFL_ERR_SIZE_TOO_LARGE
} xcflError;

typedef enum {
	XCFL_IMAGE_BASE_TYPE_UNDEFINED=-1,
	XCFL_IMAGE_BASE_TYPE_RGB,
	XCFL_IMAGE_BASE_TYPE_GRAYSCALE,
	XCFL_IMAGE_BASE_TYPE_INDEXED
} xcflImageBaseType;
#define XCFL_IMAGE_BASE_TYPE_FIRST	XCFL_IMAGE_BASE_TYPE_RGB
#define XCFL_IMAGE_BASE_TYPE_LAST	XCFL_IMAGE_BASE_TYPE_INDEXED

typedef enum {
	XCFL_IMAGE_TYPE_UNDEFINED=-1,
	XCFL_IMAGE_TYPE_RGB,
	XCFL_IMAGE_TYPE_RGB_ALPHA,
	XCFL_IMAGE_TYPE_GRAYSCALE,
	XCFL_IMAGE_TYPE_GRAYSCALE_ALPHA,
	XCFL_IMAGE_TYPE_INDEXED,
	XCFL_IMAGE_TYPE_INDEXED_ALPHA
} xcflImageType;
#define XCFL_IMAGE_TYPE_FIRST	XCFL_IMAGE_TYPE_RGB
#define XCFL_IMAGE_TYPE_LAST	XCFL_IMAGE_TYPE_INDEXED_ALPHA

typedef enum {
	XCFL_COMPOSITE_NORMAL,
	XCFL_COMPOSITE_DISSOLVE,
	XCFL_COMPOSITE_BEHIND,
	XCFL_COMPOSITE_MULTIPLY,
	XCFL_COMPOSITE_SCREEN,
	XCFL_COMPOSITE_OVERLAY,
	XCFL_COMPOSITE_DIFFERENCE,
	XCFL_COMPOSITE_ADDITION,
	XCFL_COMPOSITE_SUBTRACT,
	XCFL_COMPOSITE_DARKEN,
	XCFL_COMPOSITE_LIGHTEN,
	XCFL_COMPOSITE_HUE,
	XCFL_COMPOSITE_SATURATION,
	XCFL_COMPOSITE_COLOR,
	XCFL_COMPOSITE_VALUE,
	XCFL_COMPOSITE_DIVIDE,
	XCFL_COMPOSITE_DODGE,
	XCFL_COMPOSITE_BURN,
	XCFL_COMPOSITE_HARDLIGHT,
	XCFL_COMPOSITE_SOFTLIGHT,
	XCFL_COMPOSITE_GRAIN_EXTRACT,
	XCFL_COMPOSITE_GRAIN_MERGE,
	XCFL_COMPOSITE_COLOR_ERASE,
	XCFL_COMPOSITE_ERASE,
	XCFL_COMPOSITE_REPLACE,
	XCFL_COMPOSITE_ANTI_ERASE
} xcflCompositeMode;
#define XCFL_COMPOSITE_FIRST	XCFL_COMPOSITE_NORMAL
#define XCFL_COMPOSITE_LAST		XCFL_COMPOSITE_ANTI_ERASE

typedef enum {
	XCFL_COMPRESSION_UNCOMPRESSED,
	XCFL_COMPRESSION_RLE
} xcflCompression;
#define XCFL_COMPRESSION_FIRST	XCFL_COMPRESSION_UNCOMPRESSED
#define XCFL_COMPRESSION_LAST	XCFL_COMPRESSION_RLE

typedef enum {
	XCFL_RESOLUTION_UNDEFINED,
	XCFL_RESOLUTION_INCH,
	XCFL_RESOLUTION_MILLIMETER,
	XCFL_RESOLUTION_POINT,
	XCFL_RESOLUTION_PICA
} xcflResolutionUnit;
#define XCFL_RESOLUTION_FIRST	XCFL_RESOLUTION_UNDEFINED
#define XCFL_RESOLUTION_LAST	XCFL_RESOLUTION_PICA

typedef struct {
	xcflInt X;
	xcflInt Y;
} xcflPoint;

typedef struct {
	xcflInt Left;
	xcflInt Top;
	xcflInt Right;
	xcflInt Bottom;
} xcflRectangle;

typedef struct {
	xcflUInt8 Red;
	xcflUInt8 Green;
	xcflUInt8 Blue;
} xcflRGB24;

#define XCFL_OPACITY_OPAQUE	255


XCFL_EXPORT(xcflUInt32) xcflGetVersion(void);
XCFL_EXPORT(const char*) xcflGetVersionString(void);
#ifdef XCFL_WCHAR_SUPPORT
XCFL_EXPORT(const xcflWChar*) xcflGetVersionStringW(void);
#endif

#define XCFL_SIGNATURE_BYTES 14
#define XCFL_SIGNATURE_INVALID			(-1)
#define XCFL_SIGNATURE_UNKNOWN_VERSION	(-2)
XCFL_EXPORT(xcflInt) xcflCheckSignature(const void *pData,xcflSize Size);
XCFL_EXPORT(xcflUInt) xcflGetImageTypePixelBytes(xcflImageType Type);

typedef XCFL_CALLBACK_TYPE(xcflBool,xcflProgressCallback)(
						struct xcflProgress_tag *pProgress,int Pos,int Max);
typedef struct xcflProgress_tag {
	xcflProgressCallback pCallback;
	void *pClientData;
} xcflProgress;
XCFL_EXPORT(xcflError) xcflProgress_Init(xcflProgress *pProgress,
										 xcflProgressCallback pCallback,
										 void *pClientData);
XCFL_EXPORT(xcflBool) xcflProgress_Call(xcflProgress *pProgress,int Pos,int Max);


#ifdef __cplusplus
}
#endif


#include "xcfl_memory.h"
#include "xcfl_source.h"
#include "xcfl_stdsource.h"
#include "xcfl_destination.h"
#include "xcfl_stddest.h"
#include "xcfl_pixelbuffer.h"
#include "xcfl_property.h"
#include "xcfl_channel.h"
#include "xcfl_layer.h"
#include "xcfl_image.h"
#include "xcfl_compress.h"
#include "xcfl_decompress.h"


#endif	/* ndef XCFL_H */
