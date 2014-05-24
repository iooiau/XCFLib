/******************************************************************************
*                                                                             *
*    ifxcf.c                                Copyright(c) 2010-2013 itow,y.    *
*                                                                             *
******************************************************************************/

/*
  ifxcf - GIMP XCF format Susie plugin library
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


#include <windows.h>
#include "../XCFLib/xcfl.h"


#define IFXCF_EXPORT(type) __declspec(dllexport) type __stdcall

#define SPI_ERR_SUCCESS			0
#define SPI_ERR_NOT_IMPLEMENTED	(-1)
#define SPI_ERR_ABORT			1
#define SPI_ERR_UNKNOWN_FORMAT	2
#define SPI_ERR_DATA_BROKEN		3
#define SPI_ERR_MEMORY_ALLOC	4
#define SPI_ERR_MEMORY			5
#define SPI_ERR_FILE_READ		6
#define SPI_ERR_INTERNAL		8

#include <pshpack1.h>

struct PictureInfo {
	long left,top;
	long width;
	long height;
	WORD x_density;
	WORD y_density;
	short colorDepth;
	HLOCAL hInfo;
};

#include <poppack.h>

typedef int (__stdcall *ProgressCallbackFunc)(int nNum,int nDenom,long lData);

typedef struct {
	ProgressCallbackFunc pCallback;
	long Param;
} ProgressInfo;




static int XCFLibErrorToSPIError(xcflError Err)
{
	switch (Err) {
	case XCFL_ERR_SUCCESS:				return SPI_ERR_SUCCESS;
	case XCFL_ERR_USER_ABORT:			return SPI_ERR_ABORT;
	case XCFL_ERR_MEMORY_ALLOC:			return SPI_ERR_MEMORY_ALLOC;
	case XCFL_ERR_MEMORY_LOCK:			return SPI_ERR_MEMORY;
	case XCFL_ERR_READ:					return SPI_ERR_FILE_READ;
	case XCFL_ERR_SET_POSITION:			return SPI_ERR_FILE_READ;
	case XCFL_ERR_INVALID_FORMAT:		return SPI_ERR_UNKNOWN_FORMAT;
	case XCFL_ERR_BAD_FORMAT:			return SPI_ERR_DATA_BROKEN;
	case XCFL_ERR_UNSUPPORTED_FORMAT:	return SPI_ERR_UNKNOWN_FORMAT;
	}
	return SPI_ERR_INTERNAL;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
#ifdef XCFL_MEMORY_DEBUG
	if (fdwReason==DLL_PROCESS_ATTACH) {
		xcflMemoryDebugInit();
	}
#endif

	return TRUE;
}


IFXCF_EXPORT(int) GetPluginInfo(int infono,LPSTR buf,int buflen)
{
	static const char *InfoList[] = {
		"00IN",
		"XCF import plugin ver.0.3.1 Copyright(c) 2010-2013 itow,y.",
		"*.xcf;*.xcf.gz;*.xcfgz;*.xcf.bz2;*.xcfbz2",
		"GIMP XCF",
	};

	if (infono<0 || infono>=sizeof(InfoList)/sizeof(char*)
			|| buf==NULL || buflen<=0)
		return 0;

	lstrcpynA(buf,InfoList[infono],buflen);
	return lstrlenA(buf);
}


IFXCF_EXPORT(int) IsSupported(LPSTR filename,DWORD dw)
{
	BOOL fFile=(dw&0xFFFF0000)==0;
	xcflSource *pSource;
	xcflError Err;
	xcflInt Version;

	if (fFile) {
		HANDLE hFile=(HANDLE)dw;

		Err=xcflSource_CreateHandleSource(&pSource,hFile,XCFL_FALSE);
	} else {
		const void *pBuffer=(const void*)dw;

		if (IsBadReadPtr(pBuffer,2048))
			return 0;
		Err=xcflSource_CreateMemorySource(&pSource,pBuffer,2048);
	}
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	Err=xcflCheckSourceSignature(pSource,&Version);

	xcflSource_Delete(&pSource);

	if (Err==XCFL_ERR_INVALID_FORMAT && !fFile
			&& filename!=NULL && filename[0]!='\0') {
		const char *p=filename;

		while (*p!='\0') {
			if (*p=='.') {
				if (lstrcmpiA(p,".xcf.bz2")==0
						|| lstrcmpiA(p,".xcfbz2")==0
						|| lstrcmpiA(p,".xcf.gz")==0
						|| lstrcmpiA(p,".xcfgz")==0)
					return 1;
			} else if (IsDBCSLeadByteEx(CP_ACP,*p) && *(p+1)!='\0') {
				p++;
			}
			p++;
		}
	}

	return Err==XCFL_ERR_SUCCESS && Version>=0;
}


static xcflError CreateSource(xcflSource **ppSource,xcflMemoryDest **ppMemDest,
							  LPSTR buf,long len,unsigned int flag,BOOL fHeader)
{
	BOOL fFile=(flag&0x7)==0;
	HANDLE hFile=INVALID_HANDLE_VALUE;
	xcflError Err;
	BYTE Buffer[XCFL_CHECK_FILE_COMPRESSION_BYTES];
	xcflFileCompression Compression;

	*ppMemDest=NULL;

	if (fFile) {
		hFile=CreateFileA(buf,GENERIC_READ,FILE_SHARE_READ,NULL,
						  OPEN_EXISTING,0,NULL);
		if (hFile==INVALID_HANDLE_VALUE)
			return XCFL_ERR_READ;

		if (len>0) {
			LARGE_INTEGER Pos;

			Pos.QuadPart=len;
			Pos.LowPart=SetFilePointer(hFile,Pos.LowPart,&Pos.HighPart,
																FILE_BEGIN);
			if ((Pos.LowPart==INVALID_SET_FILE_POINTER
												&& GetLastError()!=NO_ERROR)
					|| Pos.QuadPart!=(LONGLONG)len) {
				CloseHandle(hFile);
				return XCFL_ERR_READ;
			}
		}

		Err=xcflSource_CreateHandleSource(ppSource,hFile,XCFL_TRUE);
		if (Err!=XCFL_ERR_SUCCESS) {
			CloseHandle(hFile);
			return Err;
		}
	} else {
		if (len<=0)
			return XCFL_ERR_INVALID_ARGUMENT;

		Err=xcflSource_CreateMemorySource(ppSource,buf,len);
		if (Err!=XCFL_ERR_SUCCESS)
			return Err;
	}

	if (xcflSource_Read(*ppSource,Buffer,XCFL_CHECK_FILE_COMPRESSION_BYTES)!=
						XCFL_CHECK_FILE_COMPRESSION_BYTES) {
		xcflSource_Delete(ppSource);
		return XCFL_ERR_READ;
	}
	xcflSource_SetPos(*ppSource,0);

	Compression=xcflCheckFileCompression(Buffer,XCFL_CHECK_FILE_COMPRESSION_BYTES);
	if (Compression==XCFL_FILE_COMPRESSION_UNKNOWN) {
		xcflSource_Delete(ppSource);
		return XCFL_ERR_INVALID_FORMAT;
	}
	if (Compression!=XCFL_FILE_COMPRESSION_UNCOMPRESSED) {
		Err=xcflMemoryDest_Create(ppMemDest);
		if (Err==XCFL_ERR_SUCCESS) {
			xcflDestination *pDest=xcflMemoryDest_GetDestination(*ppMemDest);

			if (fHeader)
				Err=xcflDecompressHeader(*ppSource,pDest,
										 XCFL_DECOMPRESS_HEADER_PROPERTIES);
			else
				Err=xcflDecompressSource(*ppSource,pDest);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflSource *pMemSource;

				Err=xcflMemoryDest_CreateSource(*ppMemDest,&pMemSource);
				if (Err==XCFL_ERR_SUCCESS) {
					xcflSource_Delete(ppSource);
					*ppSource=pMemSource;
				}
			}
			if (Err!=XCFL_ERR_SUCCESS)
				xcflMemoryDest_Delete(ppMemDest);
		}
		if (Err!=XCFL_ERR_SUCCESS) {
			xcflSource_Delete(ppSource);
			return Err;
		}
	}

	return XCFL_ERR_SUCCESS;
}


static void DeleteSource(xcflSource **ppSource,xcflMemoryDest **ppMemDest)
{
	xcflSource_Delete(ppSource);
	xcflMemoryDest_Delete(ppMemDest);
}


IFXCF_EXPORT(int) GetPictureInfo(LPSTR buf,long len,unsigned int flag,
								 struct PictureInfo *lpInfo)
{
	xcflError Err;
	xcflSource *pSource;
	xcflMemoryDest *pMemDest=NULL;
	xcflImage *pImage;

	if (buf==NULL || lpInfo==NULL || len<0 || (flag&0x7)>1)
		return SPI_ERR_INTERNAL;

	Err=CreateSource(&pSource,&pMemDest,buf,len,flag,TRUE);
	if (Err!=XCFL_ERR_SUCCESS)
		return XCFLibErrorToSPIError(Err);

	Err=xcflImage_Create(&pImage,pSource);
	if (Err==XCFL_ERR_SUCCESS) {
		Err=xcflImage_ReadProperties(pImage);
		if (Err==XCFL_ERR_SUCCESS) {
			xcflImageHeader ImageHeader;

			Err=xcflImage_GetHeader(pImage,&ImageHeader);
			if (Err==XCFL_ERR_SUCCESS) {
				xcflImageInfo ImageInfo;

				lpInfo->left=0;
				lpInfo->top=0;
				lpInfo->width=(long)ImageHeader.Width;
				lpInfo->height=(long)ImageHeader.Height;
				lpInfo->x_density=0;
				lpInfo->y_density=0;
				lpInfo->colorDepth=
					ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_RGB?24:8;
				lpInfo->hInfo=NULL;

				if (xcflImage_GetInfo(pImage,&ImageInfo)==XCFL_ERR_SUCCESS) {
					lpInfo->x_density=(WORD)(ImageInfo.XResolution+0.5f);
					lpInfo->y_density=(WORD)(ImageInfo.YResolution+0.5f);
				}
			}
		}
		xcflImage_Delete(&pImage);
	}

	DeleteSource(&pSource,&pMemDest);

	return XCFLibErrorToSPIError(Err);
}


static XCFL_CALLBACK_DECL(xcflBool,ProgressCallback)(xcflProgress *pProgress,
													 int Pos,int Max)
{
	if (Max>0) {
		ProgressInfo *pInfo=(ProgressInfo*)pProgress->pClientData;

		return pInfo->pCallback(Pos,Max,pInfo->Param)==0;
	}

	return XCFL_TRUE;
}


static xcflError ImageToDIB(xcflImage *pImage,HLOCAL *phBmInfo,HLOCAL *phBmBits,
					ProgressCallbackFunc pProgressCallback,long CallbackParam)
{
	xcflError Err;
	xcflImageHeader ImageHeader;
	HLOCAL hBmInfo=NULL,hBmBits=NULL;

	Err=xcflImage_ReadProperties(pImage);
	if (Err!=XCFL_ERR_SUCCESS)
		return Err;

	Err=xcflImage_GetHeader(pImage,&ImageHeader);
	if (Err==XCFL_ERR_SUCCESS) {
		xcflPixelBuffer *pPixelBuffer;

		Err=xcflPixelBuffer_Create(&pPixelBuffer);
		if (Err==XCFL_ERR_SUCCESS) {
			SIZE_T InfoSize;

			InfoSize=sizeof(BITMAPINFOHEADER);
			if (ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_GRAYSCALE
					|| ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_INDEXED)
				InfoSize+=256*sizeof(RGBQUAD);
			hBmInfo=LocalAlloc(LMEM_MOVEABLE,InfoSize);
			if (hBmInfo==NULL) {
				Err=XCFL_ERR_MEMORY_ALLOC;
			} else {
				BITMAPINFO *pbmi=(BITMAPINFO*)LocalLock(hBmInfo);

				if (pbmi==NULL) {
					Err=XCFL_ERR_MEMORY_LOCK;
				} else {
					xcflUInt i;
					int RowBytes;

					pbmi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
					pbmi->bmiHeader.biWidth=ImageHeader.Width;
					pbmi->bmiHeader.biHeight=ImageHeader.Height;
					pbmi->bmiHeader.biPlanes=1;
					pbmi->bmiHeader.biBitCount=
						ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_RGB?24:8;
					pbmi->bmiHeader.biCompression=BI_RGB;
					pbmi->bmiHeader.biSizeImage=0;
					pbmi->bmiHeader.biXPelsPerMeter=0;
					pbmi->bmiHeader.biYPelsPerMeter=0;
					pbmi->bmiHeader.biClrUsed=0;
					pbmi->bmiHeader.biClrImportant=0;

					if (ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_GRAYSCALE) {
						for (i=0;i<256;i++) {
							pbmi->bmiColors[i].rgbBlue=i;
							pbmi->bmiColors[i].rgbGreen=i;
							pbmi->bmiColors[i].rgbRed=i;
							pbmi->bmiColors[i].rgbReserved=0;
						}
					} else if (ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_INDEXED) {
						xcflRGB24 Colormap[256];
						xcflUInt NumColors;

						ZeroMemory(Colormap,sizeof(Colormap));
						NumColors=256;
						xcflImage_GetColormap(pImage,Colormap,&NumColors);
						for (i=0;i<NumColors;i++) {
							pbmi->bmiColors[i].rgbBlue=Colormap[i].Blue;
							pbmi->bmiColors[i].rgbGreen=Colormap[i].Green;
							pbmi->bmiColors[i].rgbRed=Colormap[i].Red;
							pbmi->bmiColors[i].rgbReserved=0;
						}
					}

					RowBytes=(ImageHeader.Width*pbmi->bmiHeader.biBitCount+31)/32*4;
					hBmBits=LocalAlloc(LMEM_MOVEABLE,RowBytes*ImageHeader.Height);
					if (hBmBits==NULL) {
						Err=XCFL_ERR_MEMORY_ALLOC;
					} else {
						void *pDIBBuffer=LocalLock(hBmBits);

						if (pDIBBuffer==NULL) {
							Err=XCFL_ERR_MEMORY_LOCK;
						} else {
							Err=xcflPixelBuffer_SetBuffer(pPixelBuffer,
								ImageHeader.Width,ImageHeader.Height,
								ImageHeader.BaseType==XCFL_IMAGE_BASE_TYPE_RGB?3:1,
								-RowBytes,pDIBBuffer);
							if (Err==XCFL_ERR_SUCCESS) {
								ProgressInfo ProgInfo;
								xcflProgress Progress;

								if (pProgressCallback!=NULL) {
									ProgInfo.pCallback=pProgressCallback;
									ProgInfo.Param=CallbackParam;
									xcflProgress_Init(&Progress,
													  ProgressCallback,
													  &ProgInfo);
								}

								Err=xcflImage_GetCompositedPixels(pImage,
									pPixelBuffer,
									pProgressCallback!=NULL?&Progress:NULL);
								if (Err==XCFL_ERR_SUCCESS)
									xcflPixelBuffer_SwapRGBOrder(pPixelBuffer);
							}

							LocalUnlock(hBmBits);
						}
					}

					LocalUnlock(hBmInfo);
				}
			}

			xcflPixelBuffer_Delete(&pPixelBuffer);
		}
	}

	if (Err!=XCFL_ERR_SUCCESS) {
		if (hBmInfo!=NULL)
			LocalFree(hBmInfo);
		if (hBmBits!=NULL)
			LocalFree(hBmBits);
	} else {
		*phBmInfo=hBmInfo;
		*phBmBits=hBmBits;
	}

	return Err;
}


IFXCF_EXPORT(int) GetPicture(LPSTR buf,long len,unsigned int flag,
							 HANDLE *pHBInfo,HANDLE *pHBm,
							 FARPROC lpProgressCallback,long lData)
{
	xcflError Err;
	xcflSource *pSource;
	xcflMemoryDest *pMemDest;
	xcflImage *pImage;

	if (buf==0 || len<0 || (flag&0x7)>1 || pHBInfo==NULL || pHBm==NULL)
		return SPI_ERR_INTERNAL;

	Err=CreateSource(&pSource,&pMemDest,buf,len,flag,FALSE);
	if (Err!=XCFL_ERR_SUCCESS)
		return XCFLibErrorToSPIError(Err);

	Err=xcflImage_Create(&pImage,pSource);
	if (Err==XCFL_ERR_SUCCESS) {
		Err=ImageToDIB(pImage,pHBInfo,pHBm,
					   (ProgressCallbackFunc)lpProgressCallback,lData);
		xcflImage_Delete(&pImage);
	}

	DeleteSource(&pSource,&pMemDest);

	return XCFLibErrorToSPIError(Err);
}


IFXCF_EXPORT(int) GetPreview(LPSTR buf,long len,unsigned int flag,
							 HANDLE *pHBInfo,HANDLE *pHBm,
							 FARPROC lpProgressCallback,long lData)
{
	return SPI_ERR_NOT_IMPLEMENTED;
}
