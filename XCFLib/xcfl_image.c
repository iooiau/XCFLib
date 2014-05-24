/******************************************************************************
*                                                                             *
*    xcfl_image.c                           Copyright(c) 2010-2013 itow,y.    *
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


struct xcflImage_tag {
	xcflUInt Status;
	xcflSource *pSource;
	xcflInt Version;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflImageBaseType BaseType;
	xcflPropertyList *pPropertyList;
	xcflOffsetList LayerOffsets;
	xcflOffsetList ChannelOffsets;
	xcflLayer *pLayer;
};

#define XCFL_IMAGE_STATUS_HEADER_READ		0x0001
#define XCFL_IMAGE_STATUS_HEADER_VALID		0x0002
#define XCFL_IMAGE_STATUS_PROPERTY_READ		0x0004
#define XCFL_IMAGE_STATUS_PROPERTY_VALID	0x0008




XCFL_EXPORT(xcflError) xcflImage_Create(xcflImage **ppImage,xcflSource *pSource)
{
	xcflImage *pImage;

	if (ppImage==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppImage=xcflNew(xcflImage);
	if (*ppImage==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pImage=*ppImage;

	pImage->Status=0;
	pImage->pSource=pSource;
	pImage->Version=0;
	pImage->Width=0;
	pImage->Height=0;
	pImage->BaseType=XCFL_IMAGE_BASE_TYPE_UNDEFINED;
	pImage->pPropertyList=NULL;
	xcflOffsetList_Init(&pImage->LayerOffsets);
	xcflOffsetList_Init(&pImage->ChannelOffsets);
	pImage->pLayer=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_Delete(xcflImage **ppImage)
{
	if (ppImage==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppImage!=NULL) {
		xcflImage *pImage=*ppImage;

		xcflPropertyList_Delete(&pImage->pPropertyList);
		xcflOffsetList_Free(&pImage->LayerOffsets);
		xcflOffsetList_Free(&pImage->ChannelOffsets);
		xcflLayer_DeleteAllSiblings(&pImage->pLayer);

		xcflDelete(*ppImage);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_ReadHeader(xcflImage *pImage)
{
	if (pImage==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_HEADER_READ)==0) {
		xcflUInt8 Buffer[XCFL_SIGNATURE_BYTES];
		xcflUInt32 Width,Height,BaseType;

		pImage->Status|=XCFL_IMAGE_STATUS_HEADER_READ;

		/* Read signature */
		if (xcflSource_Read(pImage->pSource,Buffer,XCFL_SIGNATURE_BYTES)!=
														XCFL_SIGNATURE_BYTES)
			return XCFL_ERR_READ;
		pImage->Version=xcflCheckSignature(Buffer,XCFL_SIGNATURE_BYTES);
		if (pImage->Version<0) {
			if (pImage->Version==XCFL_SIGNATURE_INVALID)
				return XCFL_ERR_INVALID_FORMAT;
			return XCFL_ERR_UNSUPPORTED_FORMAT;
		}

		if (!xcflSource_ReadUInt32(pImage->pSource,&Width)
				|| !xcflSource_ReadUInt32(pImage->pSource,&Height)
				|| !xcflSource_ReadUInt32(pImage->pSource,&BaseType))
			return XCFL_ERR_READ;
		if (Width==0 || Height==0)
			return XCFL_ERR_BAD_FORMAT;
		if (Width>XCFL_MAX_IMAGE_WIDTH || Height>=XCFL_MAX_IMAGE_HEIGHT)
			return XCFL_ERR_SIZE_TOO_LARGE;
		if (BaseType>XCFL_IMAGE_BASE_TYPE_LAST)
			return XCFL_ERR_UNSUPPORTED_FORMAT;

		pImage->Width=Width;
		pImage->Height=Height;
		pImage->BaseType=(xcflImageBaseType)BaseType;

		pImage->Status|=XCFL_IMAGE_STATUS_HEADER_VALID;
	} else if ((pImage->Status&XCFL_IMAGE_STATUS_HEADER_VALID)==0) {
		return XCFL_ERR_UNEXPECTED_CALL;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_GetHeader(xcflImage *pImage,xcflImageHeader *pHeader)
{
	if (pImage==NULL || pHeader==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_HEADER_READ)==0) {
		xcflError Err;

		Err=xcflImage_ReadHeader(pImage);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	if ((pImage->Status&XCFL_IMAGE_STATUS_HEADER_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	pHeader->Version=pImage->Version;
	pHeader->Width=pImage->Width;
	pHeader->Height=pImage->Height;
	pHeader->BaseType=pImage->BaseType;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_ReadProperties(xcflImage *pImage)
{
	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_READ)==0) {
		xcflError Err;

		if ((pImage->Status&XCFL_IMAGE_STATUS_HEADER_READ)==0) {
			Err=xcflImage_ReadHeader(pImage);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
		}

		if (pImage->pPropertyList==NULL) {
			Err=xcflPropertyList_Create(&pImage->pPropertyList);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
		}

		pImage->Status|=XCFL_IMAGE_STATUS_PROPERTY_READ;

		/* Read properties */
		Err=xcflPropertyList_Read(pImage->pPropertyList,pImage->pSource);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		/* Read layer offsets */
		Err=xcflOffsetList_Read(&pImage->LayerOffsets,pImage->pSource);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		/* Read channel offsets */
		Err=xcflOffsetList_Read(&pImage->ChannelOffsets,pImage->pSource);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		pImage->Status|=XCFL_IMAGE_STATUS_PROPERTY_VALID;
	} else if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0) {
		return XCFL_ERR_UNEXPECTED_CALL;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflImage_GetProperty(const xcflImage *pImage,xcflProperty *pProperty)
{
	if (pImage==NULL || pImage->pPropertyList==NULL || pProperty==NULL)
		return XCFL_FALSE;

	return xcflPropertyList_GetProperty(pImage->pPropertyList,pProperty);
}


XCFL_EXPORT(xcflError) xcflImage_GetInfo(const xcflImage *pImage,
										 xcflImageInfo *pInfo)
{
	xcflProperty Property;

	if (pImage==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	pInfo->Compression=XCFL_COMPRESSION_UNCOMPRESSED;
	pInfo->XResolution=0.0f;
	pInfo->YResolution=0.0f;
	pInfo->ResolutionUnit=XCFL_RESOLUTION_UNDEFINED;

	Property.Type=XCFL_PROPERTY_COMPRESSION;
	if (xcflPropertyList_GetProperty(pImage->pPropertyList,&Property)
			&& Property.Size==1)
		pInfo->Compression=(xcflCompression)*(xcflUInt8*)Property.pData;

	Property.Type=XCFL_PROPERTY_RESOLUTION;
	if (xcflPropertyList_GetProperty(pImage->pPropertyList,&Property)
			&& Property.Size==8) {
		pInfo->XResolution=xcflProperty_GetFloat(&Property,0);
		pInfo->YResolution=xcflProperty_GetFloat(&Property,1);
	}

	Property.Type=XCFL_PROPERTY_UNIT;
	if (xcflPropertyList_GetProperty(pImage->pPropertyList,&Property)
			&& Property.Size==4)
		pInfo->ResolutionUnit=
			(xcflResolutionUnit)xcflProperty_GetUInt32(&Property,0);

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_GetCompression(const xcflImage *pImage,
												xcflCompression *pCompression)
{
	xcflProperty Property;

	if (pImage==NULL || pCompression==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	*pCompression=XCFL_COMPRESSION_UNCOMPRESSED;

	Property.Type=XCFL_PROPERTY_COMPRESSION;
	if (xcflPropertyList_GetProperty(pImage->pPropertyList,&Property)
			&& Property.Size==1)
		*pCompression=(xcflCompression)*(xcflUInt8*)Property.pData;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_GetColormap(const xcflImage *pImage,
											 xcflRGB24 *pColormap,xcflUInt *pNumColors)
{
	xcflProperty Property;
	xcflUInt32 NumColors,i;
	xcflUInt8 *p;

	if (pImage==NULL || pColormap==NULL || pNumColors==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	Property.Type=XCFL_PROPERTY_COLORMAP;
	if (!xcflPropertyList_GetProperty(pImage->pPropertyList,&Property)
			|| Property.Size<=4
			|| (NumColors=xcflProperty_GetUInt32(&Property,0))==0
			|| (Property.Size-4)/3<NumColors)
		return XCFL_ERR_BAD_FORMAT;

	if (NumColors>*pNumColors)
		NumColors=*pNumColors;
	else
		*pNumColors=NumColors;

	p=(xcflUInt8*)Property.pData+4;
	for (i=0;i<NumColors;i++) {
		pColormap[i].Red=*p++;
		pColormap[i].Green=*p++;
		pColormap[i].Blue=*p++;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflInt) xcflImage_GetNumLayers(const xcflImage *pImage)
{
	if (pImage==NULL)
		return 0;

	return pImage->LayerOffsets.NumOffsets;
}


XCFL_EXPORT(xcflError) xcflImage_ReadLayer(xcflImage *pImage,
										   int Index,xcflLayer **ppLayer)
{
	xcflError Err;

	if (ppLayer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppLayer=NULL;

	if (pImage==NULL
			|| Index<0 || (xcflUInt)Index>=pImage->LayerOffsets.NumOffsets)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	Err=xcflLayer_Create(ppLayer);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	Err=xcflSource_SetPos(pImage->pSource,pImage->LayerOffsets.pOffsets[Index]);
	if (Err==XCFL_ERR_SUCCESS)
		Err=xcflLayer_ReadHeader(*ppLayer,pImage->pSource);

	if (Err!=XCFL_ERR_SUCCESS)
		xcflLayer_Delete(ppLayer);

	return Err;
}


XCFL_EXPORT(xcflError) xcflImage_ReadLayers(xcflImage *pImage)
{
	xcflError Err;
	xcflInt NumLayers;
	xcflInt i;
	xcflLayer *pRootLayer;

	if (pImage==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pImage->pLayer!=NULL)
		return XCFL_ERR_SUCCESS;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_READ)==0) {
		Err=xcflImage_ReadProperties(pImage);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	NumLayers=pImage->LayerOffsets.NumOffsets;
	if (NumLayers==0)
		return XCFL_ERR_BAD_FORMAT;

	pRootLayer=NULL;

	for (i=0;i<NumLayers;i++) {
		xcflLayer *pLayer;

		Err=xcflImage_ReadLayer(pImage,i,&pLayer);
		if (Err==XCFL_ERR_SUCCESS) {
			if (pRootLayer==NULL)
				pRootLayer=pLayer;

			if (xcflLayer_HasItemPath(pLayer)) {
				if (pRootLayer==pLayer) {
					Err=XCFL_ERR_BAD_FORMAT;
				} else {
					xcflItemPath Path;

					Err=xcflLayer_GetItemPath(pLayer,&Path);
					if (Err==XCFL_ERR_SUCCESS) {
						xcflLayer *pPrev;

						pPrev=xcflLayer_GetSibling(pRootLayer,Path.pList[0]);
						if (pPrev!=NULL) {
							xcflUInt j;

							for (j=1;j+1<Path.NumElements;j++) {
								pPrev=xcflLayer_GetChild(pPrev,Path.pList[j]);
								if (pPrev==NULL)
									break;
							}
						}
						if (pPrev!=NULL) {
							Err=xcflLayer_AppendChild(pPrev,pLayer);
						} else {
							Err=XCFL_ERR_BAD_FORMAT;
						}

						xcflItemPath_Free(&Path);
					}
				}
			} else {
				if (pLayer!=pRootLayer)
					Err=xcflLayer_AppendSibling(pRootLayer,pLayer);
			}

			if (Err!=XCFL_ERR_SUCCESS)
				xcflLayer_Delete(&pLayer);
		}

		if (Err!=XCFL_ERR_SUCCESS) {
			xcflLayer_DeleteAllSiblings(&pRootLayer);
			return Err;
		}
	}

	pImage->pLayer=pRootLayer;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflLayer*) xcflImage_GetTopLayer(const xcflImage *pImage)
{
	if (pImage==NULL)
		return NULL;

	return pImage->pLayer;
}


XCFL_EXPORT(xcflLayer*) xcflImage_GetBottomLayer(const xcflImage *pImage)
{
	if (pImage==NULL || pImage->pLayer==NULL)
		return NULL;

	return xcflLayer_GetBottomSibling(pImage->pLayer);
}


XCFL_EXPORT(xcflInt) xcflImage_GetNumChannels(const xcflImage *pImage)
{
	if (pImage==NULL)
		return 0;

	return pImage->ChannelOffsets.NumOffsets;
}


XCFL_EXPORT(xcflError) xcflImage_ReadChannel(xcflImage *pImage,
											 int Index,xcflChannel **ppChannel)
{
	xcflError Err;

	if (ppChannel==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppChannel=NULL;

	if (pImage==NULL
			|| Index<0 || (xcflUInt)Index>=pImage->ChannelOffsets.NumOffsets)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	Err=xcflChannel_Create(ppChannel);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	Err=xcflSource_SetPos(pImage->pSource,pImage->ChannelOffsets.pOffsets[Index]);
	if (Err==XCFL_ERR_SUCCESS)
		Err=xcflChannel_ReadHeader(*ppChannel,pImage->pSource);

	if (Err!=XCFL_ERR_SUCCESS)
		xcflChannel_Delete(ppChannel);

	return Err;
}


typedef struct {
	xcflLayer *pLayer;
	xcflBool LoadData;
	xcflBool LoadMask;
} xcflLayerLoadInfo;


static xcflError GetLayerLoadInfo(xcflLayer *pLayer,xcflImage *pImage,
								  xcflLayerLoadInfo *pLoadInfo,xcflInt *pInfoCount)
{
	xcflError Err;
	xcflLayerInfo LayerInfo;

	Err=xcflLayer_GetInfo(pLayer,&LayerInfo);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if ((LayerInfo.Flags&XCFL_LAYER_FLAG_VISIBLE)!=0
		&& LayerInfo.Opacity>0) {
		xcflInt i=*pInfoCount;

		pLoadInfo[i].pLayer=pLayer;
		pLoadInfo[i].LoadData=
			(LayerInfo.Flags&XCFL_LAYER_FLAG_GROUP)==0;
		pLoadInfo[i].LoadMask=XCFL_FALSE;

		if ((LayerInfo.Flags&XCFL_LAYER_FLAG_HAS_MASK)!=0
				&& (LayerInfo.Flags&XCFL_LAYER_FLAG_APPLY_MASK)!=0) {
			xcflChannelInfo MaskInfo;

			Err=xcflLayer_ReadMask(pLayer,pImage->pSource);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
			Err=xcflLayer_GetMaskInfo(pLayer,&MaskInfo);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
			if (MaskInfo.Opacity>0)
				pLoadInfo[i].LoadMask=XCFL_TRUE;
		}

		++*pInfoCount;

		if (xcflLayer_HasChild(pLayer)) {
			xcflLayer *pChild;

			pChild=xcflLayer_GetFirstChild(pLayer);
			do {
				Err=GetLayerLoadInfo(pChild,pImage,pLoadInfo,pInfoCount);
				if (Err!=XCFL_ERR_SUCCESS)
					return Err;
				pChild=xcflLayer_GetNextSibling(pChild);
			} while (pChild!=NULL);
		}
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflImage_GetCompositedPixels(xcflImage *pImage,
						xcflPixelBuffer *pPixelBuffer,xcflProgress *pProgress)
{
	xcflError Err;
	xcflPixelBufferInfo PixelBufferInfo;
	xcflLayer *pLayer;
	xcflInt NumLayers;
	xcflInt i;
	xcflLayerLoadInfo *pLayerLoadInfo;
	xcflInt LoadLayerCount;

	if (pImage==NULL || pPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_READ)==0) {
		Err=xcflImage_ReadProperties(pImage);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	if ((pImage->Status&XCFL_IMAGE_STATUS_PROPERTY_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	Err=xcflPixelBuffer_GetInfo(pPixelBuffer,&PixelBufferInfo);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if (PixelBufferInfo.Width!=pImage->Width
			|| PixelBufferInfo.Height!=pImage->Height
			|| (pImage->BaseType==XCFL_IMAGE_BASE_TYPE_RGB
				&& PixelBufferInfo.BytesPerPixel!=3
				&& PixelBufferInfo.BytesPerPixel!=4)
			|| ((pImage->BaseType==XCFL_IMAGE_BASE_TYPE_GRAYSCALE
				|| pImage->BaseType==XCFL_IMAGE_BASE_TYPE_INDEXED)
				&& PixelBufferInfo.BytesPerPixel!=1
				&& PixelBufferInfo.BytesPerPixel!=2))
		return XCFL_ERR_INVALID_ARGUMENT;

	NumLayers=pImage->LayerOffsets.NumOffsets;
	if (NumLayers==0)
		return XCFL_ERR_BAD_FORMAT;

	Err=xcflImage_ReadLayers(pImage);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	pLayerLoadInfo=(xcflLayerLoadInfo*)xcflMemoryAlloc(NumLayers*sizeof(xcflLayerLoadInfo));
	if (pLayerLoadInfo==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pLayer=pImage->pLayer;
	LoadLayerCount=0;
	do {
		Err=GetLayerLoadInfo(pLayer,pImage,pLayerLoadInfo,&LoadLayerCount);
		if (Err!=XCFL_ERR_SUCCESS) {
			xcflMemoryFree(pLayerLoadInfo);
			return Err;
		}

		pLayer=xcflLayer_GetNextSibling(pLayer);
	} while (pLayer!=NULL);

	xcflPixelBuffer_ClearPixels(pPixelBuffer);

	if (LoadLayerCount>0) {
		xcflCompression Compression;

		xcflImage_GetCompression(pImage,&Compression);

		for (i=0;i<LoadLayerCount;i++) {
			if (pLayerLoadInfo[i].LoadData) {
				Err=xcflLayer_ReadData(pLayerLoadInfo[i].pLayer,
									   pImage->pSource,Compression);
				if (Err!=XCFL_ERR_SUCCESS)
					break;
			}

			if (pLayerLoadInfo[i].LoadMask) {
				Err=xcflLayer_ReadMaskData(pLayerLoadInfo[i].pLayer,
										   pImage->pSource,Compression);
				if (Err!=XCFL_ERR_SUCCESS)
					break;
			}

			if (pProgress!=NULL) {
				if (!xcflProgress_Call(pProgress,i,LoadLayerCount-1)) {
					Err=XCFL_ERR_USER_ABORT;
					break;
				}
			}
		}

		if (Err==XCFL_ERR_SUCCESS) {
			pLayer=xcflLayer_GetBottomSibling(pImage->pLayer);
			do {
				if (xcflLayer_IsGroup(pLayer)
						|| xcflLayer_IsDataLoaded(pLayer)) {
					Err=xcflLayer_CompositePixels(pLayer,pPixelBuffer,NULL);
					if (Err!=XCFL_ERR_SUCCESS)
						break;
				}

				pLayer=xcflLayer_GetUpperSibling(pLayer);
			} while (pLayer!=NULL);
		}
	}

	xcflMemoryFree(pLayerLoadInfo);

	return Err;
}
