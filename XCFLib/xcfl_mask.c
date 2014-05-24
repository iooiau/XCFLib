/******************************************************************************
*                                                                             *
*    xcfl_mask.c                            Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_mask.h"




xcflError xcflApplyMask(void *pPixels,xcflSize BytesPerPixel,
						const void *pMask,xcflUInt Opacity,xcflSize Length)
{
	xcflSize AlphaOffset,i;
	const xcflUInt8 *p;
	xcflUInt8 *q;

	if (pPixels==NULL || BytesPerPixel<1 || pMask==NULL || Length==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	AlphaOffset=BytesPerPixel-1;

	p=(const xcflUInt8*)pMask;
	q=(xcflUInt8*)pPixels;

	if (Opacity==XCFL_OPACITY_OPAQUE) {
		for (i=0;i<Length;i++) {
			q[AlphaOffset]=xcflDivideBy255(q[AlphaOffset]*(*p));
			p++;
			q+=BytesPerPixel;
		}
	} else if (Opacity>0) {
		const xcflUInt Alpha=255-Opacity;

		for (i=0;i<Length;i++) {
			q[AlphaOffset]=xcflDivideBy255(q[AlphaOffset]*Alpha+(*p)*Opacity);
			p++;
			q+=BytesPerPixel;
		}
	}

	return XCFL_ERR_SUCCESS;
}


xcflError xcflCombineMask(void *pDstPixels,xcflSize BytesPerPixel,
						  const void *pSrcPixels,
						  const void *pMask,xcflUInt Opacity,xcflSize Length)
{
	xcflSize AlphaOffset,i;
	const xcflUInt8 *p,*m;
	xcflUInt8 *q;

	if (pDstPixels==NULL || BytesPerPixel<1
			|| pSrcPixels==NULL || pMask==NULL || Length==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	AlphaOffset=BytesPerPixel-1;

	p=(const xcflUInt8*)pSrcPixels;
	m=(const xcflUInt8*)pMask;
	q=(xcflUInt8*)pDstPixels;

	if (Opacity==XCFL_OPACITY_OPAQUE) {
		for (i=0;i<Length;i++) {
			switch (BytesPerPixel) {
			case 4:	q[2]=p[2];
			case 3:	q[1]=p[1];
			case 2:	q[0]=p[0];
			}
			q[AlphaOffset]=xcflDivideBy255(p[AlphaOffset]*(*m));
			m++;
			p+=BytesPerPixel;
			q+=BytesPerPixel;
		}
	} else if (Opacity>0) {
		const xcflUInt Alpha=255-Opacity;

		for (i=0;i<Length;i++) {
			switch (BytesPerPixel) {
			case 4:	q[2]=p[2];
			case 3:	q[1]=p[1];
			case 2:	q[0]=p[0];
			}
			q[AlphaOffset]=xcflDivideBy255(p[AlphaOffset]*Alpha+(*m)*Opacity);
			m++;
			p+=BytesPerPixel;
			q+=BytesPerPixel;
		}
	}

	return XCFL_ERR_SUCCESS;
}
