/******************************************************************************
*                                                                             *
*    xcfl_channel.c                         Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_offset.h"
#include "xcfl_hierarchy.h"


struct xcflChannel_tag {
	xcflUInt Status;
	xcflUInt32 Width;
	xcflUInt32 Height;
	char *pName;
	xcflPropertyList *pPropertyList;
	xcflOffsetList HierarchyOffsets;
	xcflHierarchy *pHierarchy;
};

#define XCFL_CHANNEL_STATUS_HEADER_READ		0x0001
#define XCFL_CHANNEL_STATUS_HEADER_VALID	0x0002




XCFL_EXPORT(xcflError) xcflChannel_Create(xcflChannel **ppChannel)
{
	xcflChannel *pChannel;

	if (ppChannel==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppChannel=xcflNew(xcflChannel);
	if (*ppChannel==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pChannel=*ppChannel;

	pChannel->Status=0;
	pChannel->Width=0;
	pChannel->Height=0;
	pChannel->pName=NULL;
	pChannel->pPropertyList=NULL;
	xcflOffsetList_Init(&pChannel->HierarchyOffsets);
	pChannel->pHierarchy=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflChannel_Delete(xcflChannel **ppChannel)
{
	if (ppChannel==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppChannel!=NULL) {
		xcflChannel *pChannel=*ppChannel;

		xcflMemoryFree(pChannel->pName);
		xcflPropertyList_Delete(&pChannel->pPropertyList);
		xcflOffsetList_Free(&pChannel->HierarchyOffsets);
		xcflHierarchy_Delete(&pChannel->pHierarchy);

		xcflDelete(*ppChannel);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflChannel_ReadHeader(xcflChannel *pChannel,
											  xcflSource *pSource)
{
	xcflUInt32 Width,Height,NameLength;
	xcflError Err;

	if (pChannel==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pChannel->Status&XCFL_CHANNEL_STATUS_HEADER_READ)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (!xcflSource_ReadUInt32(pSource,&Width)
			|| !xcflSource_ReadUInt32(pSource,&Height))
		return XCFL_ERR_READ;

	if (Width==0 || Height==0)
		return XCFL_ERR_BAD_FORMAT;
	if (Width>XCFL_MAX_IMAGE_WIDTH || Height>XCFL_MAX_IMAGE_HEIGHT)
		return XCFL_ERR_SIZE_TOO_LARGE;

	pChannel->Status|=XCFL_CHANNEL_STATUS_HEADER_READ;
	pChannel->Width=Width;
	pChannel->Height=Height;

	/* Read channel name */
	if (!xcflSource_ReadUInt32(pSource,&NameLength))
		return XCFL_ERR_READ;
	if (NameLength>0) {
		pChannel->pName=(char*)xcflMemoryAlloc(NameLength+1);
		if (pChannel->pName==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
		if (xcflSource_Read(pSource,pChannel->pName,NameLength)!=NameLength) {
			xcflMemoryFree(pChannel->pName);
			pChannel->pName=NULL;
			return XCFL_ERR_READ;
		}
		pChannel->pName[NameLength]='\0';
	}

	/* Read properties */
	if (pChannel->pPropertyList==NULL) {
		Err=xcflPropertyList_Create(&pChannel->pPropertyList);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}
	Err=xcflPropertyList_Read(pChannel->pPropertyList,pSource);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	Err=xcflOffsetList_Read(&pChannel->HierarchyOffsets,pSource);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if (pChannel->HierarchyOffsets.NumOffsets==0)
		return XCFL_ERR_BAD_FORMAT;

	pChannel->Status|=XCFL_CHANNEL_STATUS_HEADER_VALID;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflChannel_GetProperty(const xcflChannel *pChannel,
											  xcflProperty *pProperty)
{
	if (pChannel==NULL || pChannel->pPropertyList==NULL || pProperty==NULL)
		return XCFL_FALSE;

	return xcflPropertyList_GetProperty(pChannel->pPropertyList,pProperty);
}


XCFL_EXPORT(xcflError) xcflChannel_GetInfo(const xcflChannel *pChannel,
										   xcflChannelInfo *pInfo)
{
	xcflProperty Property;

	if (pChannel==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pChannel->Status&XCFL_CHANNEL_STATUS_HEADER_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	pInfo->Width=pChannel->Width;
	pInfo->Height=pChannel->Height;
	pInfo->Opacity=XCFL_OPACITY_OPAQUE;
	pInfo->Flags=0;

	Property.Type=XCFL_PROPERTY_OPACITY;
	if (xcflPropertyList_GetProperty(pChannel->pPropertyList,&Property)
			&& Property.Size==4)
		pInfo->Opacity=xcflProperty_GetUInt32(&Property,0);

	Property.Type=XCFL_PROPERTY_VISIBLE;
	if (xcflPropertyList_GetProperty(pChannel->pPropertyList,&Property)
			&& Property.Size==4) {
		if (xcflProperty_GetUInt32(&Property,0)!=0)
			pInfo->Flags|=XCFL_CHANNEL_FLAG_VISIBLE;
	}

	if (xcflPropertyList_HasProperty(pChannel->pPropertyList,
									 XCFL_PROPERTY_ACTIVE_CHANNEL))
		pInfo->Flags|=XCFL_CHANNEL_FLAG_ACTIVE;

	Property.Type=XCFL_PROPERTY_SELECTION;
	if (xcflPropertyList_GetProperty(pChannel->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_CHANNEL_FLAG_SELECTION;

	Property.Type=XCFL_PROPERTY_SHOW_MASKED;
	if (xcflPropertyList_GetProperty(pChannel->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_CHANNEL_FLAG_SHOW_MASKED;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(const char*) xcflChannel_GetName(const xcflChannel *pChannel)
{
	if (pChannel==NULL)
		return NULL;

	return pChannel->pName;
}


XCFL_EXPORT(xcflError) xcflChannel_ReadData(xcflChannel *pChannel,
											xcflSource *pSource,
											xcflCompression Compression)
{
	if (pChannel==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pChannel->Status&XCFL_CHANNEL_STATUS_HEADER_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pChannel->pHierarchy==NULL) {
		xcflError Err;

		Err=xcflSource_SetPos(pSource,pChannel->HierarchyOffsets.pOffsets[0]);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		Err=xcflHierarchy_Create(&pChannel->pHierarchy,Compression);
		if (Err==XCFL_ERR_SUCCESS) {
			Err=xcflHierarchy_ReadHeader(pChannel->pHierarchy,pSource);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflHierarchyInfo Info;

				Err=xcflHierarchy_GetInfo(pChannel->pHierarchy,&Info);
				if (Err==XCFL_ERR_SUCCESS) {
					if (Info.Width!=pChannel->Width
							|| Info.Height!=pChannel->Height) {
						Err=XCFL_ERR_BAD_FORMAT;
					} else {
						Err=xcflHierarchy_ReadData(pChannel->pHierarchy,pSource);
					}
				}
			}

			if (Err!=XCFL_ERR_SUCCESS)
				xcflHierarchy_Delete(&pChannel->pHierarchy);
		}

		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflChannel_IsDataLoaded(const xcflChannel *pChannel)
{
	if (pChannel==NULL || pChannel->pHierarchy==NULL)
		return XCFL_FALSE;

	return xcflHierarchy_IsDataLoaded(pChannel->pHierarchy);
}


XCFL_EXPORT(void*) xcflChannel_GetRow(const xcflChannel *pChannel,xcflUInt Top)
{
	if (pChannel==NULL || Top>=pChannel->Height)
		return NULL;

	if (pChannel->pHierarchy==NULL)
		return NULL;

	return xcflHierarchy_GetRow(pChannel->pHierarchy,Top);
}
