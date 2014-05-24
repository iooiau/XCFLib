/******************************************************************************
*                                                                             *
*    xcfl_channel.h                         Copyright(c) 2010-2013 itow,y.    *
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


#ifndef XCFL_CHANNEL_H
#define XCFL_CHANNEL_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct xcflChannel_tag xcflChannel;

typedef struct {
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflUInt32 Opacity;
	xcflUInt Flags;
} xcflChannelInfo;

#define XCFL_CHANNEL_FLAG_VISIBLE		0x0001
#define XCFL_CHANNEL_FLAG_ACTIVE		0x0002
#define XCFL_CHANNEL_FLAG_SELECTION		0x0004
#define XCFL_CHANNEL_FLAG_SHOW_MASKED	0x0008


XCFL_EXPORT(xcflError) xcflChannel_Create(xcflChannel **ppChannel);
XCFL_EXPORT(xcflError) xcflChannel_Delete(xcflChannel **ppChannel);
XCFL_EXPORT(xcflError) xcflChannel_ReadHeader(xcflChannel *pChannel,
											  xcflSource *pSource);
XCFL_EXPORT(xcflBool) xcflChannel_GetProperty(const xcflChannel *pChannel,
											  xcflProperty *pProperty);
XCFL_EXPORT(const char*) xcflChannel_GetName(const xcflChannel *pChannel);
XCFL_EXPORT(xcflError) xcflChannel_GetInfo(const xcflChannel *pChannel,
										   xcflChannelInfo *pInfo);
XCFL_EXPORT(xcflError) xcflChannel_ReadData(xcflChannel *pChannel,
											xcflSource *pSource,
											xcflCompression Compression);
XCFL_EXPORT(xcflBool) xcflChannel_IsDataLoaded(const xcflChannel *pChannel);
XCFL_EXPORT(void*) xcflChannel_GetRow(const xcflChannel *pChannel,xcflUInt Top);


#ifdef __cplusplus
}
#endif


#endif	/* ndef XCFL_CHANNEL_H */
