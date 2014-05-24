/******************************************************************************
*                                                                             *
*    xcfl_layer.c                           Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_hierarchy.h"
#include "xcfl_composite.h"
#include "xcfl_mask.h"


struct xcflLayer_tag {
	xcflUInt Status;
	xcflUInt32 Width;
	xcflUInt32 Height;
	xcflImageType Type;
	char *pName;
	xcflPropertyList *pPropertyList;
	xcflUInt32 HierarchyOffset;
	xcflUInt32 LayerMaskOffset;
	xcflHierarchy *pHierarchy;
	xcflChannel *pLayerMask;
	xcflLayer *pPrevLayer;
	xcflLayer *pNextLayer;
	xcflLayer *pChildLayer;
};

#define XCFL_LAYER_STATUS_HEADER_READ	0x0001
#define XCFL_LAYER_STATUS_HEADER_VALID	0x0002




XCFL_EXPORT(xcflError) xcflLayer_Create(xcflLayer **ppLayer)
{
	xcflLayer *pLayer;

	if (ppLayer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	*ppLayer=xcflNew(xcflLayer);
	if (*ppLayer==NULL)
		return XCFL_ERR_MEMORY_ALLOC;

	pLayer=*ppLayer;

	pLayer->Status=0;
	pLayer->Width=0;
	pLayer->Height=0;
	pLayer->Type=XCFL_IMAGE_TYPE_UNDEFINED;
	pLayer->pName=NULL;
	pLayer->pPropertyList=NULL;
	pLayer->HierarchyOffset=0;
	pLayer->LayerMaskOffset=0;
	pLayer->pHierarchy=NULL;
	pLayer->pLayerMask=NULL;
	pLayer->pPrevLayer=NULL;
	pLayer->pNextLayer=NULL;
	pLayer->pChildLayer=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLayer_Delete(xcflLayer **ppLayer)
{
	if (ppLayer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppLayer!=NULL) {
		xcflLayer *pLayer=*ppLayer;

		xcflMemoryFree(pLayer->pName);
		xcflPropertyList_Delete(&pLayer->pPropertyList);
		xcflHierarchy_Delete(&pLayer->pHierarchy);
		xcflChannel_Delete(&pLayer->pLayerMask);

		if (pLayer->pChildLayer!=NULL) {
			xcflLayer *pChild,*pNext;

			pChild=pLayer->pChildLayer;
			do {
				pNext=pChild->pNextLayer;
				xcflLayer_Delete(&pChild);
				pChild=pNext;
			} while (pChild!=NULL);
		}

		xcflDelete(*ppLayer);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLayer_DeleteAllSiblings(xcflLayer **ppLayer)
{
	if (ppLayer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (*ppLayer!=NULL) {
		xcflLayer *pLayer=xcflLayer_GetFirstSibling(*ppLayer);

		do {
			xcflLayer *pNext=pLayer->pNextLayer;

			xcflLayer_Delete(&pLayer);
			pLayer=pNext;
		} while (pLayer!=NULL);

		*ppLayer=NULL;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLayer_ReadHeader(xcflLayer *pLayer,
											xcflSource *pSource)
{
	xcflUInt32 Width,Height,Type,NameLength;
	xcflError Err;

	if (pLayer==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLayer->Status&XCFL_LAYER_STATUS_HEADER_READ)!=0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (!xcflSource_ReadUInt32(pSource,&Width)
			|| !xcflSource_ReadUInt32(pSource,&Height)
			|| !xcflSource_ReadUInt32(pSource,&Type))
		return XCFL_ERR_READ;

	if (Width==0 || Height==0)
		return XCFL_ERR_BAD_FORMAT;
	if (Width>XCFL_MAX_IMAGE_WIDTH || Height>XCFL_MAX_IMAGE_HEIGHT)
		return XCFL_ERR_SIZE_TOO_LARGE;
	if (Type>XCFL_IMAGE_TYPE_LAST)
		return XCFL_ERR_UNSUPPORTED_FORMAT;

	pLayer->Status|=XCFL_LAYER_STATUS_HEADER_READ;

	pLayer->Width=Width;
	pLayer->Height=Height;
	pLayer->Type=(xcflImageType)Type;

	/* Read layer name */
	if (!xcflSource_ReadUInt32(pSource,&NameLength))
		return XCFL_ERR_READ;
	if (NameLength>0) {
		pLayer->pName=(char*)xcflMemoryAlloc(NameLength+1);
		if (pLayer->pName==NULL)
			return XCFL_ERR_MEMORY_ALLOC;
		if (xcflSource_Read(pSource,pLayer->pName,NameLength)!=NameLength) {
			xcflMemoryFree(pLayer->pName);
			pLayer->pName=NULL;
			return XCFL_ERR_READ;
		}
		pLayer->pName[NameLength]='\0';
	}

	/* Read properties */
	if (pLayer->pPropertyList==NULL) {
		Err=xcflPropertyList_Create(&pLayer->pPropertyList);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}
	Err=xcflPropertyList_Read(pLayer->pPropertyList,pSource);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if (!xcflSource_ReadUInt32(pSource,&pLayer->HierarchyOffset)
			|| !xcflSource_ReadUInt32(pSource,&pLayer->LayerMaskOffset))
		return XCFL_ERR_READ;

	pLayer->Status|=XCFL_LAYER_STATUS_HEADER_VALID;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflLayer_GetProperty(const xcflLayer *pLayer,
											xcflProperty *pProperty)
{
	if (pLayer==NULL || pLayer->pPropertyList==NULL || pProperty==NULL)
		return XCFL_FALSE;

	return xcflPropertyList_GetProperty(pLayer->pPropertyList,pProperty);
}


XCFL_EXPORT(xcflError) xcflLayer_GetInfo(const xcflLayer *pLayer,
										 xcflLayerInfo *pInfo)
{
	xcflProperty Property;

	if (pLayer==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLayer->Status&XCFL_LAYER_STATUS_HEADER_VALID)==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	pInfo->Width=pLayer->Width;
	pInfo->Height=pLayer->Height;
	pInfo->Type=pLayer->Type;
	pInfo->Mode=XCFL_COMPOSITE_NORMAL;
	pInfo->Opacity=XCFL_OPACITY_OPAQUE;
	pInfo->XOffset=0;
	pInfo->YOffset=0;
	pInfo->Flags=0;

	if (pLayer->LayerMaskOffset!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_HAS_MASK;

	Property.Type=XCFL_PROPERTY_MODE;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4)
		pInfo->Mode=(xcflCompositeMode)xcflProperty_GetUInt32(&Property,0);

	Property.Type=XCFL_PROPERTY_OPACITY;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4)
		pInfo->Opacity=xcflProperty_GetUInt32(&Property,0);

	Property.Type=XCFL_PROPERTY_VISIBLE;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4) {
		if (xcflProperty_GetUInt32(&Property,0)!=0)
			pInfo->Flags|=XCFL_LAYER_FLAG_VISIBLE;
	} else {
		pInfo->Flags|=XCFL_LAYER_FLAG_VISIBLE;
	}

	if (xcflPropertyList_HasProperty(pLayer->pPropertyList,
									 XCFL_PROPERTY_ACTIVE_LAYER))
		pInfo->Flags|=XCFL_LAYER_FLAG_ACTIVE;

	Property.Type=XCFL_PROPERTY_PRESERVE_TRANSPARENCY;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_PRESERVE_TRANSPARENCY;

	Property.Type=XCFL_PROPERTY_APPLY_MASK;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_APPLY_MASK;
	else if (pLayer->LayerMaskOffset!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_APPLY_MASK;

	Property.Type=XCFL_PROPERTY_EDIT_MASK;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_EDIT_MASK;

	Property.Type=XCFL_PROPERTY_SHOW_MASK;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==4 && xcflProperty_GetUInt32(&Property,0)!=0)
		pInfo->Flags|=XCFL_LAYER_FLAG_SHOW_MASK;

	Property.Type=XCFL_PROPERTY_OFFSETS;
	if (xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property)
			&& Property.Size==8) {
		pInfo->XOffset=xcflProperty_GetInt32(&Property,0);
		pInfo->YOffset=xcflProperty_GetInt32(&Property,1);
	}

	if (xcflPropertyList_HasProperty(pLayer->pPropertyList,
									 XCFL_PROPERTY_GROUP_ITEM))
		pInfo->Flags|=XCFL_LAYER_FLAG_GROUP;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(const char*) xcflLayer_GetName(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	return pLayer->pName;
}


XCFL_EXPORT(xcflError) xcflLayer_ReadData(xcflLayer *pLayer,
										  xcflSource *pSource,
										  xcflCompression Compression)
{
	if (pLayer==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLayer->Status&XCFL_LAYER_STATUS_HEADER_VALID)==0
			|| pLayer->HierarchyOffset==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pLayer->pHierarchy==NULL) {
		xcflError Err;

		Err=xcflSource_SetPos(pSource,pLayer->HierarchyOffset);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		Err=xcflHierarchy_Create(&pLayer->pHierarchy,Compression);
		if (Err==XCFL_ERR_SUCCESS) {
			Err=xcflHierarchy_ReadHeader(pLayer->pHierarchy,pSource);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflHierarchyInfo Info;

				Err=xcflHierarchy_GetInfo(pLayer->pHierarchy,&Info);
				if (Err==XCFL_ERR_SUCCESS) {
					if (Info.Width!=pLayer->Width
							|| Info.Height!=pLayer->Height
							|| Info.BytesPerPixel!=
								xcflGetImageTypePixelBytes(pLayer->Type)) {
						Err=XCFL_ERR_BAD_FORMAT;
					} else {
						Err=xcflHierarchy_ReadData(pLayer->pHierarchy,pSource);
					}
				}
			}

			if (Err!=XCFL_ERR_SUCCESS)
				xcflHierarchy_Delete(&pLayer->pHierarchy);
		}

		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflLayer_IsDataLoaded(const xcflLayer *pLayer)
{
	if (pLayer==NULL || pLayer->pHierarchy==NULL)
		return XCFL_FALSE;

	return xcflHierarchy_IsDataLoaded(pLayer->pHierarchy);
}


XCFL_EXPORT(xcflError) xcflLayer_ReadMask(xcflLayer *pLayer,
										  xcflSource *pSource)
{
	if (pLayer==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLayer->Status&XCFL_LAYER_STATUS_HEADER_VALID)==0
			|| pLayer->LayerMaskOffset==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pLayer->pLayerMask==NULL) {
		xcflError Err;

		Err=xcflSource_SetPos(pSource,pLayer->LayerMaskOffset);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		Err=xcflChannel_Create(&pLayer->pLayerMask);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;

		Err=xcflChannel_ReadHeader(pLayer->pLayerMask,pSource);
		if (Err==XCFL_ERR_SUCCESS) {
			xcflChannelInfo Info;

			Err=xcflChannel_GetInfo(pLayer->pLayerMask,&Info);
			if (Err==XCFL_ERR_SUCCESS) {
				if (Info.Width!=pLayer->Width
						|| Info.Height!=pLayer->Height)
					Err=XCFL_ERR_BAD_FORMAT;
			}
		}

		if (Err!=XCFL_ERR_SUCCESS)
			xcflChannel_Delete(&pLayer->pLayerMask);
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflLayer_GetMaskInfo(const xcflLayer *pLayer,
											 xcflChannelInfo *pInfo)
{
	if (pLayer==NULL || pInfo==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pLayer->pLayerMask==NULL)
		return XCFL_ERR_UNEXPECTED_CALL;

	return xcflChannel_GetInfo(pLayer->pLayerMask,pInfo);
}


XCFL_EXPORT(xcflError) xcflLayer_ReadMaskData(xcflLayer *pLayer,
											  xcflSource *pSource,
											  xcflCompression Compression)
{
	if (pLayer==NULL || pSource==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if ((pLayer->Status&XCFL_LAYER_STATUS_HEADER_VALID)==0
			|| pLayer->LayerMaskOffset==0)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (pLayer->pLayerMask==NULL) {
		xcflError Err;

		Err=xcflLayer_ReadMask(pLayer,pSource);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	return xcflChannel_ReadData(pLayer->pLayerMask,pSource,Compression);
}


XCFL_EXPORT(xcflError) xcflLayer_CompositePixels(xcflLayer *pLayer,
												 xcflPixelBuffer *pPixelBuffer,
												 const xcflPoint *pSrcPos)
{
	xcflError Err;
	xcflBool IsGroup;
	xcflLayerInfo LayerInfo;
	xcflInt XOffset,YOffset;
	xcflPixelBufferInfo PixelBufferInfo;

	if (pLayer==NULL || pPixelBuffer==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	IsGroup=xcflLayer_IsGroup(pLayer);

	if (pLayer->pHierarchy==NULL && !IsGroup)
		return XCFL_ERR_UNEXPECTED_CALL;

	if (IsGroup && pLayer->pChildLayer==NULL)
		return XCFL_ERR_SUCCESS;

	Err=xcflLayer_GetInfo(pLayer,&LayerInfo);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	XOffset=LayerInfo.XOffset;
	YOffset=LayerInfo.YOffset;
	if (pSrcPos!=NULL) {
		XOffset-=pSrcPos->X;
		YOffset-=pSrcPos->Y;
	}

	Err=xcflPixelBuffer_GetInfo(pPixelBuffer,&PixelBufferInfo);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	if ((LayerInfo.Flags&XCFL_LAYER_FLAG_VISIBLE)!=0
			&& LayerInfo.Opacity>0
			&& XOffset<(xcflInt)PixelBufferInfo.Width
			&& YOffset<(xcflInt)PixelBufferInfo.Height
			&& XOffset+(xcflInt)LayerInfo.Width>0
			&& YOffset+(xcflInt)LayerInfo.Height>0) {
		xcflRectangle DstRect;
		xcflUInt SrcLeft,SrcTop,Width,Height,LayerPixelBytes,y;
		xcflPixelBuffer *pGroupPixelBuffer=NULL;
		xcflBool ApplyMask=XCFL_FALSE;
		xcflChannelInfo MaskInfo;
		void *pMaskedBuffer=NULL;

		if (XOffset<0) {
			SrcLeft=-XOffset;
			DstRect.Left=0;
		} else {
			SrcLeft=0;
			DstRect.Left=XOffset;
		}
		if (YOffset<0) {
			SrcTop=-YOffset;
			DstRect.Top=0;
		} else {
			SrcTop=0;
			DstRect.Top=YOffset;
		}
		DstRect.Right=XOffset+LayerInfo.Width;
		if (DstRect.Right>(xcflInt)PixelBufferInfo.Width)
			DstRect.Right=PixelBufferInfo.Width;
		DstRect.Bottom=YOffset+LayerInfo.Height;
		if (DstRect.Bottom>(xcflInt)PixelBufferInfo.Height)
			DstRect.Bottom=PixelBufferInfo.Height;

		Width=DstRect.Right-DstRect.Left;
		Height=DstRect.Bottom-DstRect.Top;
		LayerPixelBytes=xcflGetImageTypePixelBytes(pLayer->Type);

		if (IsGroup) {
			xcflPoint Pos;
			xcflLayer *pChild;

			Err=xcflPixelBuffer_Create(&pGroupPixelBuffer);
			if (Err!=XCFL_ERR_SUCCESS)
				return Err;
			Err=xcflPixelBuffer_Allocate(pGroupPixelBuffer,
				LayerInfo.Width,LayerInfo.Height,LayerPixelBytes);
			if (Err!=XCFL_ERR_SUCCESS) {
				xcflPixelBuffer_Delete(&pGroupPixelBuffer);
				return Err;
			}

			xcflPixelBuffer_ClearPixels(pGroupPixelBuffer);

			Pos.X=LayerInfo.XOffset;
			Pos.Y=LayerInfo.YOffset;

			pChild=xcflLayer_GetLastChild(pLayer);
			while (pChild!=NULL) {
				if (xcflLayer_IsGroup(pChild)
						|| xcflLayer_IsDataLoaded(pChild)) {
					Err=xcflLayer_CompositePixels(pChild,pGroupPixelBuffer,&Pos);
					if (Err!=XCFL_ERR_SUCCESS) {
						xcflPixelBuffer_Delete(&pGroupPixelBuffer);
						return Err;
					}
				}
				pChild=pChild->pPrevLayer;
			}
		}

		if (pLayer->pLayerMask!=NULL
				&& (LayerInfo.Flags&XCFL_LAYER_FLAG_APPLY_MASK)!=0
				&& xcflChannel_IsDataLoaded(pLayer->pLayerMask)) {
			Err=xcflChannel_GetInfo(pLayer->pLayerMask,&MaskInfo);
			if (Err==XCFL_ERR_SUCCESS) {
				if (MaskInfo.Opacity>0)
					ApplyMask=XCFL_TRUE;
			}
		}

		if (ApplyMask) {
			pMaskedBuffer=xcflMemoryAlloc(Width*LayerPixelBytes);
			if (pMaskedBuffer==NULL) {
				xcflPixelBuffer_Delete(&pGroupPixelBuffer);
				return XCFL_ERR_MEMORY_ALLOC;
			}
		}

		for (y=0;y<Height;y++) {
			xcflUInt8 *pSrc,*pDst;

			if (IsGroup)
				pSrc=(xcflUInt8*)xcflPixelBuffer_GetRow(pGroupPixelBuffer,SrcTop+y);
			else
				pSrc=(xcflUInt8*)xcflHierarchy_GetRow(pLayer->pHierarchy,SrcTop+y);
			pDst=(xcflUInt8*)xcflPixelBuffer_GetPixelPtr(pPixelBuffer,
														 DstRect.Left,DstRect.Top+y);
			if (pSrc==NULL || pDst==NULL) {
				Err=XCFL_ERR_INTERNAL;
				break;
			}

			pSrc+=SrcLeft*LayerPixelBytes;

			if (ApplyMask) {
				xcflUInt8 *pMask;

				pMask=(xcflUInt8*)xcflChannel_GetRow(pLayer->pLayerMask,SrcTop+y);
				if (pMask==NULL) {
					Err=XCFL_ERR_INTERNAL;
					break;
				}
				pMask+=SrcLeft;
				xcflCombineMask(pMaskedBuffer,LayerPixelBytes,
								pSrc,pMask,MaskInfo.Opacity,Width);
				pSrc=pMaskedBuffer;
			}

			Err=xcflCompositePixels(pDst,PixelBufferInfo.BytesPerPixel,
									pSrc,pLayer->Type,
									LayerInfo.Mode,LayerInfo.Opacity,Width);
			if (Err!=XCFL_ERR_SUCCESS)
				break;
		}

		xcflMemoryFree(pMaskedBuffer);
		xcflPixelBuffer_Delete(&pGroupPixelBuffer);
	}

	return Err;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetPrevSibling(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	return pLayer->pPrevLayer;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetNextSibling(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	return pLayer->pNextLayer;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetFirstSibling(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	while (pLayer->pPrevLayer!=NULL)
		pLayer=pLayer->pPrevLayer;

	return (xcflLayer*)pLayer;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetLastSibling(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	while (pLayer->pNextLayer!=NULL)
		pLayer=pLayer->pNextLayer;

	return (xcflLayer*)pLayer;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetSibling(const xcflLayer *pLayer,
											 int Index)
{
	xcflLayer *pSibling;
	int i;

	if (pLayer==NULL)
		return NULL;

	if (Index==0)
		return (xcflLayer*)pLayer;

	if (Index>0) {
		pSibling=pLayer->pNextLayer;
		for (i=1;i<Index;i++) {
			if (pSibling==NULL)
				break;
			pSibling=pSibling->pNextLayer;
		}
	} else {
		pSibling=pLayer->pPrevLayer;
		for (i=-1;i>Index;i--) {
			if (pSibling==NULL)
				break;
			pSibling=pSibling->pPrevLayer;
		}
	}

	return pSibling;
}


XCFL_EXPORT(xcflError) xcflLayer_AppendSibling(xcflLayer *pLayer,
											   xcflLayer *pSibling)
{
	xcflLayer *pPrev;

	if (pLayer==NULL || pSibling==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	pPrev=pLayer;
	while (pPrev->pNextLayer!=NULL)
		pPrev=pPrev->pNextLayer;
	pPrev->pNextLayer=pSibling;
	pSibling->pPrevLayer=pPrev;
	pSibling->pNextLayer=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflLayer_IsGroup(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return XCFL_FALSE;

	return xcflPropertyList_HasProperty(pLayer->pPropertyList,
										XCFL_PROPERTY_GROUP_ITEM);
}


XCFL_EXPORT(xcflBool) xcflLayer_HasChild(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return XCFL_FALSE;

	return pLayer->pChildLayer!=NULL;
}


XCFL_EXPORT(int) xcflLayer_GetChildCount(const xcflLayer *pLayer)
{
	xcflLayer *pChild;
	int i;

	if (pLayer==NULL)
		return 0;

	pChild=pLayer->pChildLayer;
	for (i=0;pChild!=NULL;i++)
		pChild=pChild->pNextLayer;

	return i;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetFirstChild(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return NULL;

	return pLayer->pChildLayer;
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetLastChild(const xcflLayer *pLayer)
{
	if (pLayer==NULL || pLayer->pChildLayer==NULL)
		return NULL;

	return xcflLayer_GetLastSibling(pLayer->pChildLayer);
}


XCFL_EXPORT(xcflLayer*) xcflLayer_GetChild(const xcflLayer *pLayer,
										   int Index)
{
	xcflLayer *pChild;
	int i;

	if (pLayer==NULL || Index<0)
		return NULL;

	pChild=pLayer->pChildLayer;
	for (i=0;i<Index;i++) {
		if (pChild==NULL)
			return NULL;
		pChild=pChild->pNextLayer;
	}

	return pChild;
}


XCFL_EXPORT(xcflError) xcflLayer_AppendChild(xcflLayer *pLayer,
											 xcflLayer *pChild)
{
	if (pLayer==NULL || pChild==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (pLayer->pChildLayer==NULL) {
		pLayer->pChildLayer=pChild;
		pChild->pPrevLayer=NULL;
	} else {
		xcflLayer *pPrev;

		pPrev=pLayer->pChildLayer;
		while (pPrev->pNextLayer!=NULL)
			pPrev=pPrev->pNextLayer;
		pPrev->pNextLayer=pChild;
		pChild->pPrevLayer=pPrev;
	}
	pChild->pNextLayer=NULL;

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflBool) xcflLayer_HasItemPath(const xcflLayer *pLayer)
{
	if (pLayer==NULL)
		return XCFL_FALSE;

	return xcflPropertyList_HasProperty(pLayer->pPropertyList,
										XCFL_PROPERTY_ITEM_PATH);
}


XCFL_EXPORT(xcflError) xcflLayer_GetItemPath(const xcflLayer *pLayer,
											 xcflItemPath *pPath)
{
	xcflProperty Property;
	xcflUInt NumElements,i;
	xcflError Err;

	if (pLayer==NULL || pPath==NULL)
		return XCFL_ERR_INVALID_ARGUMENT;

	xcflItemPath_Init(pPath);

	Property.Type=XCFL_PROPERTY_ITEM_PATH;
	if (!xcflPropertyList_GetProperty(pLayer->pPropertyList,&Property))
		return XCFL_ERR_UNEXPECTED_CALL;

	NumElements=Property.Size/4;
	if (NumElements==0)
		return XCFL_ERR_BAD_FORMAT;

	Err=xcflItemPath_Allocate(pPath,NumElements);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	for (i=0;i<NumElements;i++)
		pPath->pList[i]=xcflProperty_GetUInt32(&Property,i);

	return XCFL_ERR_SUCCESS;
}
