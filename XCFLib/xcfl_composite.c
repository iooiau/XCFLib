/******************************************************************************
*                                                                             *
*    xcfl_composite.c                       Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_composite.h"
#include "xcfl_colorspace.h"


#define MULTIPLY(a,b) xcflDivideBy255((a)*(b))

#define BLEND(c1,a1,c2,a2) ((xcflUInt8)xcflDivideBy255((c1)*(a1)+(c2)*(a2)))
#define BLEND_ALPHA(a,b) ((xcflUInt8)((a)+MULTIPLY(255-(a),b)))
#define COMPOSITE(c1,a1,c2,a2,a) \
	((xcflUInt8)(((c1)*(a1)+(c2)*(a2))/(a)))




static void NormalComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			if (Opacity<255) {
				Beta=255-Opacity;
				for (i=0;i<Length;i++) {
					*q=BLEND(*q,Beta,*p,Opacity);
					p++;
					q++;
				}
			} else {
				xcflMemoryCopy(q,p,Length);
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			if (Opacity<255) {
				Beta=255-Opacity;
				for (i=0;i<Length;i++) {
					int a1,a;

					a1=MULTIPLY(q[1],Beta);
					a=a1+Opacity;
					q[0]=COMPOSITE(q[0],a1,*p,Opacity,a);
					q[1]=BLEND_ALPHA(q[1],Opacity);
					p++;
					q+=2;
				}
			} else {
				for (i=0;i<Length;i++) {
					*q++=*p++;
					*q++=255;
				}
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				int a1,a2,a;

				a2=MULTIPLY(p[1],Opacity);
				a1=MULTIPLY(q[1],255-a2);
				a=a1+a2;
				if (a!=0) {
					q[0]=COMPOSITE(q[0],a1,p[0],a2,a);
					q[1]=BLEND_ALPHA(q[1],a2);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			if (Opacity<255) {
				Beta=255-Opacity;
				for (i=0;i<Length;i++) {
					q[0]=BLEND(q[0],Beta,p[0],Opacity);
					q[1]=BLEND(q[1],Beta,p[1],Opacity);
					q[2]=BLEND(q[2],Beta,p[2],Opacity);
					p+=3;
					q+=3;
				}
			} else {
				xcflMemoryCopy(q,p,Length*3);
			}
		} else if (SrcBytesPerPixel==4) {
			if (Opacity<255) {
				for (i=0;i<Length;i++) {
					Alpha=MULTIPLY(p[3],Opacity);
					Beta=255-Alpha;
					q[0]=BLEND(q[0],Beta,p[0],Alpha);
					q[1]=BLEND(q[1],Beta,p[1],Alpha);
					q[2]=BLEND(q[2],Beta,p[2],Alpha);
					p+=4;
					q+=3;
				}
			} else {
				for (i=0;i<Length;i++) {
					Alpha=p[3];
					Beta=255-Alpha;
					q[0]=BLEND(q[0],Beta,p[0],Alpha);
					q[1]=BLEND(q[1],Beta,p[1],Alpha);
					q[2]=BLEND(q[2],Beta,p[2],Alpha);
					p+=4;
					q+=3;
				}
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			if (Opacity<255) {
				Beta=255-Opacity;
				for (i=0;i<Length;i++) {
					int a1,a;

					a1=MULTIPLY(q[3],Beta);
					a=a1+Opacity;
					q[0]=COMPOSITE(q[0],a1,p[0],Opacity,a);
					q[1]=COMPOSITE(q[1],a1,p[1],Opacity,a);
					q[2]=COMPOSITE(q[2],a1,p[2],Opacity,a);
					q[3]=BLEND_ALPHA(q[3],Opacity);
					p+=3;
					q+=4;
				}
			} else {
				for (i=0;i<Length;i++) {
					*q++=*p++;
					*q++=*p++;
					*q++=*p++;
					*q++=255;
				}
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				int a1,a2,a;

				a2=MULTIPLY(p[3],Opacity);
				a1=MULTIPLY(q[3],255-a2);
				a=a1+a2;
				if (a!=0) {
					q[0]=COMPOSITE(q[0],a1,p[0],a2,a);
					q[1]=COMPOSITE(q[1],a1,p[1],a2,a);
					q[2]=COMPOSITE(q[2],a1,p[2],a2,a);
					q[3]=BLEND_ALPHA(q[3],a2);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void MultiplyComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define MULTIPLY_BLEND(c1,a1,c2,a2) BLEND(c1,a1,MULTIPLY(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=MULTIPLY_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=MULTIPLY_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=MULTIPLY_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=MULTIPLY(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=MULTIPLY_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=MULTIPLY_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=MULTIPLY_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=MULTIPLY_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=MULTIPLY_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=MULTIPLY_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=MULTIPLY_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=MULTIPLY_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=MULTIPLY_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=MULTIPLY(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=MULTIPLY(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=MULTIPLY(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void ScreenComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define SCREEN(a,b) (255-MULTIPLY(255-(a),255-(b)))
#define SCREEN_BLEND(c1,a1,c2,a2) BLEND(c1,a1,SCREEN(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=SCREEN_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=SCREEN_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=SCREEN_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=SCREEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=SCREEN_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=SCREEN_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=SCREEN_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=SCREEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SCREEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SCREEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=SCREEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SCREEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SCREEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=SCREEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=SCREEN(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=SCREEN(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void OverlayComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define OVERLAY(a,b) MULTIPLY(a,(a)+MULTIPLY(2*(b),255-(a)))
#define OVERLAY_BLEND(c1,a1,c2,a2) BLEND(c1,a1,OVERLAY(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=OVERLAY_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=OVERLAY_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=OVERLAY_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=OVERLAY(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=OVERLAY_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=OVERLAY_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=OVERLAY_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=OVERLAY_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=OVERLAY_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=OVERLAY_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=OVERLAY_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=OVERLAY_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=OVERLAY_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=OVERLAY(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=OVERLAY(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=OVERLAY(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void DifferenceComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define DIFFERENCE(a,b) xcflAbs((a)-(b))
#define DIFFERENCE_BLEND(c1,a1,c2,a2) BLEND(c1,a1,DIFFERENCE(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=DIFFERENCE_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=DIFFERENCE_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=DIFFERENCE_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=DIFFERENCE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=DIFFERENCE_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=DIFFERENCE_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=DIFFERENCE_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=DIFFERENCE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DIFFERENCE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DIFFERENCE_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=DIFFERENCE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DIFFERENCE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DIFFERENCE_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=DIFFERENCE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=DIFFERENCE(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=DIFFERENCE(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void AdditionComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define ADDITION(a,b) xcflMin((a)+(b),255)
#define ADDITION_BLEND(c1,a1,c2,a2) BLEND(c1,a1,ADDITION(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=ADDITION_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=ADDITION_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=ADDITION_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=ADDITION(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=ADDITION_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=ADDITION_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=ADDITION_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=ADDITION_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=ADDITION_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=ADDITION_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=ADDITION_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=ADDITION_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=ADDITION_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=ADDITION(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=ADDITION(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=ADDITION(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void SubtractComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define SUBTRACT(a,b) xcflMax((a)-(b),0)
#define SUBTRACT_BLEND(c1,a1,c2,a2) BLEND(c1,a1,SUBTRACT(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=SUBTRACT_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=SUBTRACT_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=SUBTRACT_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=SUBTRACT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=SUBTRACT_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=SUBTRACT_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=SUBTRACT_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=SUBTRACT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SUBTRACT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SUBTRACT_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=SUBTRACT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SUBTRACT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SUBTRACT_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=SUBTRACT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=SUBTRACT(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=SUBTRACT(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void DarkenComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define DARKEN(a,b) xcflMin(a,b)
#define DARKEN_BLEND(c1,a1,c2,a2) BLEND(c1,a1,DARKEN(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=DARKEN_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=DARKEN_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=DARKEN_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=DARKEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=DARKEN_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=DARKEN_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=DARKEN_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=DARKEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DARKEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DARKEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=DARKEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DARKEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DARKEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=DARKEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=DARKEN(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=DARKEN(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void LightenComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define LIGHTEN(a,b) xcflMax(a,b)
#define LIGHTEN_BLEND(c1,a1,c2,a2) BLEND(c1,a1,LIGHTEN(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=LIGHTEN_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=LIGHTEN_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=LIGHTEN_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=LIGHTEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=LIGHTEN_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=LIGHTEN_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=LIGHTEN_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=LIGHTEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=LIGHTEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=LIGHTEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=LIGHTEN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=LIGHTEN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=LIGHTEN_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=LIGHTEN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=LIGHTEN(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=LIGHTEN(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void GrayComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	if (DstBytesPerPixel==2) {
		xcflSize i;
		const xcflUInt8 *p;
		xcflUInt8 *q;
		xcflUInt Alpha,Beta;

		p=(const xcflUInt8*)pSrcData;
		q=(xcflUInt8*)pDstData;

		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=BLEND(q[0],Beta,*p,Opacity);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				if (Alpha!=0) {
					Beta=BLEND_ALPHA(q[1],Alpha);
					if (Alpha==Beta) {
						q[0]=p[0];
					} else {
						int a1,a2;

						a1=Alpha*255/Beta;
						a2=255-a1;
						q[0]=BLEND(p[0],a1,q[0],a2);
					}
				}
				p+=2;
				q+=2;
			}
		}
	} else {
		NormalComposite(pDstData,DstBytesPerPixel,
						pSrcData,SrcBytesPerPixel,
						Opacity,Length);
	}
}


static void HueComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int h,s,v,r1,g1,b1,r2,g2,b2;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
	case 2:
		GrayComposite(pDstData,DstBytesPerPixel,
					  pSrcData,SrcBytesPerPixel,
					  Opacity,Length);
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				h=p[0];
				s=p[1];
				v=p[2];
				xcflRGBToHSVInt(&h,&s,&v);
				if (s>0) {
					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					r2=h;
					xcflHSVToRGBInt(&r2,&g2,&b2);
					q[0]=BLEND(r1,Beta,r2,Opacity);
					q[1]=BLEND(g1,Beta,g2,Opacity);
					q[2]=BLEND(b1,Beta,b2,Opacity);
				}
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				if (Alpha!=0) {
					h=p[0];
					s=p[1];
					v=p[2];
					xcflRGBToHSVInt(&h,&s,&v);
					if (s>0) {
						r1=r2=q[0];
						g1=g2=q[1];
						b1=b2=q[2];
						xcflRGBToHSVInt(&r2,&g2,&b2);
						r2=h;
						xcflHSVToRGBInt(&r2,&g2,&b2);
						Beta=255-Alpha;
						q[0]=BLEND(r1,Beta,r2,Alpha);
						q[1]=BLEND(g1,Beta,g2,Alpha);
						q[2]=BLEND(b1,Beta,b2,Alpha);
					}
				}
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(q[3],Opacity);
				Beta=255-Alpha;
				h=p[0];
				s=p[1];
				v=p[2];
				xcflRGBToHSVInt(&h,&s,&v);
				if (s>0) {
					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					r2=h;
					xcflHSVToRGBInt(&r2,&g2,&b2);
					q[0]=BLEND(r1,Beta,r2,Alpha);
					q[1]=BLEND(g1,Beta,g2,Alpha);
					q[2]=BLEND(b1,Beta,b2,Alpha);
				}
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					h=p[0];
					s=p[1];
					v=p[2];
					xcflRGBToHSVInt(&h,&s,&v);
					if (s>0) {
						int a1,a2,a;

						r1=r2=q[0];
						g1=g2=q[1];
						b1=b2=q[2];
						a1=q[3]*(255-Alpha);
						a2=p[3]*Alpha;
						a=a1+a2;
						xcflRGBToHSVInt(&r2,&g2,&b2);
						r2=h;
						xcflHSVToRGBInt(&r2,&g2,&b2);
						q[0]=COMPOSITE(r1,a1,r2,a2,a);
						q[1]=COMPOSITE(g1,a1,g2,a2,a);
						q[2]=COMPOSITE(b1,a1,b2,a2,a);
					}
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void SaturationComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int r1,g1,b1,r2,g2,b2;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
	case 2:
		GrayComposite(pDstData,DstBytesPerPixel,
					  pSrcData,SrcBytesPerPixel,
					  Opacity,Length);
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSVInt(&r2,&g2,&b2);
				g2=xcflRGBToHSVInt_S(p[0],p[1],p[2]);
				xcflHSVToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Opacity);
				q[1]=BLEND(g1,Beta,g2,Opacity);
				q[2]=BLEND(b1,Beta,b2,Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				if (Alpha!=0) {
					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					g2=xcflRGBToHSVInt_S(p[0],p[1],p[2]);
					xcflHSVToRGBInt(&r2,&g2,&b2);
					Beta=255-Alpha;
					q[0]=BLEND(r1,Beta,r2,Alpha);
					q[1]=BLEND(g1,Beta,g2,Alpha);
					q[2]=BLEND(b1,Beta,b2,Alpha);
				}
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(q[3],Opacity);
				Beta=255-Alpha;
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSVInt(&r2,&g2,&b2);
				g2=xcflRGBToHSVInt_S(p[0],p[1],p[2]);
				xcflHSVToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Alpha);
				q[1]=BLEND(g1,Beta,g2,Alpha);
				q[2]=BLEND(b1,Beta,b2,Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a;

					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					g2=xcflRGBToHSVInt_S(p[0],p[1],p[2]);
					xcflHSVToRGBInt(&r2,&g2,&b2);
					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					q[0]=COMPOSITE(r1,a1,r2,a2,a);
					q[1]=COMPOSITE(g1,a1,g2,a2,a);
					q[2]=COMPOSITE(b1,a1,b2,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void ColorComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int h,s,v,r1,g1,b1,r2,g2,b2;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
	case 2:
		GrayComposite(pDstData,DstBytesPerPixel,
					  pSrcData,SrcBytesPerPixel,
					  Opacity,Length);
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSLInt(&r2,&g2,&b2);
				h=p[0];
				s=p[1];
				v=p[2];
				xcflRGBToHSLInt(&h,&s,&v);
				r2=h;
				g2=s;
				xcflHSLToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Opacity);
				q[1]=BLEND(g1,Beta,g2,Opacity);
				q[2]=BLEND(b1,Beta,b2,Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				if (Alpha!=0) {
					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSLInt(&r2,&g2,&b2);
					h=p[0];
					s=p[1];
					v=p[2];
					xcflRGBToHSLInt(&h,&s,&v);
					r2=h;
					g2=s;
					xcflHSLToRGBInt(&r2,&g2,&b2);
					Beta=255-Alpha;
					q[0]=BLEND(r1,Beta,r2,Alpha);
					q[1]=BLEND(g1,Beta,g2,Alpha);
					q[2]=BLEND(b1,Beta,b2,Alpha);
				}
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(q[3],Opacity);
				Beta=255-Alpha;
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSLInt(&r2,&g2,&b2);
				h=p[0];
				s=p[1];
				v=p[2];
				xcflRGBToHSLInt(&h,&s,&v);
				r2=h;
				g2=s;
				xcflHSLToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Alpha);
				q[1]=BLEND(g1,Beta,g2,Alpha);
				q[2]=BLEND(b1,Beta,b2,Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a;

					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSLInt(&r2,&g2,&b2);
					h=p[0];
					s=p[1];
					v=p[2];
					xcflRGBToHSLInt(&h,&s,&v);
					r2=h;
					g2=s;
					xcflHSLToRGBInt(&r2,&g2,&b2);
					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					q[0]=COMPOSITE(r1,a1,r2,a2,a);
					q[1]=COMPOSITE(g1,a1,g2,a2,a);
					q[2]=COMPOSITE(b1,a1,b2,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void ValueComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int r1,g1,b1,r2,g2,b2;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
	case 2:
		GrayComposite(pDstData,DstBytesPerPixel,
					  pSrcData,SrcBytesPerPixel,
					  Opacity,Length);
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSVInt(&r2,&g2,&b2);
				b2=xcflRGBToHSVInt_V(p[0],p[1],p[2]);
				xcflHSVToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Opacity);
				q[1]=BLEND(g1,Beta,g2,Opacity);
				q[2]=BLEND(b1,Beta,b2,Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				if (Alpha!=0) {
					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					b2=xcflRGBToHSVInt_V(p[0],p[1],p[2]);
					xcflHSVToRGBInt(&r2,&g2,&b2);
					Beta=255-Alpha;
					q[0]=BLEND(r1,Beta,r2,Alpha);
					q[1]=BLEND(g1,Beta,g2,Alpha);
					q[2]=BLEND(b1,Beta,b2,Alpha);
				}
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(q[3],Opacity);
				Beta=255-Alpha;
				r1=r2=q[0];
				g1=g2=q[1];
				b1=b2=q[2];
				xcflRGBToHSVInt(&r2,&g2,&b2);
				b2=xcflRGBToHSVInt_V(p[0],p[1],p[2]);
				xcflHSVToRGBInt(&r2,&g2,&b2);
				q[0]=BLEND(r1,Beta,r2,Alpha);
				q[1]=BLEND(g1,Beta,g2,Alpha);
				q[2]=BLEND(b1,Beta,b2,Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a;

					r1=r2=q[0];
					g1=g2=q[1];
					b1=b2=q[2];
					xcflRGBToHSVInt(&r2,&g2,&b2);
					b2=xcflRGBToHSVInt_V(p[0],p[1],p[2]);
					xcflHSVToRGBInt(&r2,&g2,&b2);
					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					q[0]=COMPOSITE(r1,a1,r2,a2,a);
					q[1]=COMPOSITE(g1,a1,g2,a2,a);
					q[2]=COMPOSITE(b1,a1,b2,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void DivideComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define DIVIDE(a,b) (Temp=((a)<<8)/((b)+1),xcflMin(Temp,255))
#define DIVIDE_BLEND(c1,a1,c2,a2) BLEND(c1,a1,DIVIDE(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=DIVIDE_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=DIVIDE_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=DIVIDE_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=DIVIDE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=DIVIDE_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=DIVIDE_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=DIVIDE_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=DIVIDE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DIVIDE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DIVIDE_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=DIVIDE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DIVIDE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DIVIDE_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=DIVIDE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=DIVIDE(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=DIVIDE(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void DodgeComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define DODGE(a,b) (Temp=((a)<<8)/(256-(b)),xcflMin(Temp,255))
#define DODGE_BLEND(c1,a1,c2,a2) BLEND(c1,a1,DODGE(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=DODGE_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=DODGE_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=DODGE_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=DODGE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=DODGE_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=DODGE_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=DODGE_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=DODGE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DODGE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DODGE_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=DODGE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=DODGE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=DODGE_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=DODGE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=DODGE(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=DODGE(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void BurnComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define BURN(a,b) (Temp=255-((255-(a))<<8)/((b)+1),xcflClamp8(Temp))
#define BURN_BLEND(c1,a1,c2,a2) BLEND(c1,a1,BURN(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=BURN_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=BURN_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=BURN_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=BURN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=BURN_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=BURN_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=BURN_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=BURN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=BURN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=BURN_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=BURN_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=BURN_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=BURN_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=BURN(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=BURN(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=BURN(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void HardlightComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define HARDLIGHT(a,b) \
	(((b)>128? \
	 (Temp=255-(((255-a)*(255-((b-128)<<1)))>>8)): \
	 (Temp=((a)*((b)<<1))>>8) \
	),xcflMin(Temp,255))
#define HARDLIGHT_BLEND(c1,a1,c2,a2) BLEND(c1,a1,HARDLIGHT(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	unsigned int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=HARDLIGHT_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=HARDLIGHT_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=HARDLIGHT_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=HARDLIGHT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=HARDLIGHT_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=HARDLIGHT_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=HARDLIGHT_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=HARDLIGHT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=HARDLIGHT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=HARDLIGHT_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=HARDLIGHT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=HARDLIGHT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=HARDLIGHT_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=HARDLIGHT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=HARDLIGHT(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=HARDLIGHT(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void SoftlightComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define SOFTLIGHT(a,b) \
	(MULTIPLY(255-(a),MULTIPLY(a,b))+MULTIPLY(a,255-(MULTIPLY(255-(a),255-(b)))))
#define SOFTLIGHT_BLEND(c1,a1,c2,a2) BLEND(c1,a1,SOFTLIGHT(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=SOFTLIGHT_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=SOFTLIGHT_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=SOFTLIGHT_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=SOFTLIGHT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=SOFTLIGHT_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=SOFTLIGHT_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=SOFTLIGHT_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=SOFTLIGHT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SOFTLIGHT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SOFTLIGHT_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=SOFTLIGHT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=SOFTLIGHT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=SOFTLIGHT_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=SOFTLIGHT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=SOFTLIGHT(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=SOFTLIGHT(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void GrainExtractComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define GRAIN_EXTRACT(a,b) (Temp=(a)-(b)+128,xcflClamp8(Temp))
#define GRAIN_EXTRACT_BLEND(c1,a1,c2,a2) BLEND(c1,a1,GRAIN_EXTRACT(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=GRAIN_EXTRACT_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=GRAIN_EXTRACT_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=GRAIN_EXTRACT_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=GRAIN_EXTRACT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=GRAIN_EXTRACT_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=GRAIN_EXTRACT_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=GRAIN_EXTRACT_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=GRAIN_EXTRACT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=GRAIN_EXTRACT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=GRAIN_EXTRACT_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=GRAIN_EXTRACT_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=GRAIN_EXTRACT_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=GRAIN_EXTRACT_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=GRAIN_EXTRACT(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=GRAIN_EXTRACT(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=GRAIN_EXTRACT(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static void GrainMergeComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
#define GRAIN_MERGE(a,b) (Temp=(a)+(b)-128,xcflClamp8(Temp))
#define GRAIN_MERGE_BLEND(c1,a1,c2,a2) BLEND(c1,a1,GRAIN_MERGE(c1,c2),a2)

	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;
	xcflUInt Alpha,Beta;
	int Temp;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				*q=GRAIN_MERGE_BLEND(*q,Beta,*p,Opacity);
				p++;
				q++;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[1],Opacity);
				Beta=255-Alpha;
				*q=GRAIN_MERGE_BLEND(*q,Beta,p[0],Alpha);
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[1]);
				Beta=255-Alpha;
				q[0]=GRAIN_MERGE_BLEND(q[0],Beta,*p,Alpha);
				p++;
				q+=2;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[1],p[1]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[1]*(255-Alpha);
					a2=p[1]*Alpha;
					a=a1+a2;
					c=GRAIN_MERGE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
				}
				p+=2;
				q+=2;
			}
		}
		break;

	case 3:
		if (SrcBytesPerPixel==3) {
			Beta=255-Opacity;
			for (i=0;i<Length;i++) {
				q[0]=GRAIN_MERGE_BLEND(q[0],Beta,p[0],Opacity);
				q[1]=GRAIN_MERGE_BLEND(q[1],Beta,p[1],Opacity);
				q[2]=GRAIN_MERGE_BLEND(q[2],Beta,p[2],Opacity);
				p+=3;
				q+=3;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(p[3],Opacity);
				Beta=255-Alpha;
				q[0]=GRAIN_MERGE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=GRAIN_MERGE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=GRAIN_MERGE_BLEND(q[2],Beta,p[2],Alpha);
				p+=4;
				q+=3;
			}
		}
		break;

	case 4:
		if (SrcBytesPerPixel==3) {
			for (i=0;i<Length;i++) {
				Alpha=xcflMin(Opacity,q[3]);
				Beta=255-Alpha;
				q[0]=GRAIN_MERGE_BLEND(q[0],Beta,p[0],Alpha);
				q[1]=GRAIN_MERGE_BLEND(q[1],Beta,p[1],Alpha);
				q[2]=GRAIN_MERGE_BLEND(q[2],Beta,p[2],Alpha);
				p+=3;
				q+=4;
			}
		} else if (SrcBytesPerPixel==4) {
			for (i=0;i<Length;i++) {
				Alpha=MULTIPLY(xcflMin(q[3],p[3]),Opacity);
				if (Alpha!=0) {
					int a1,a2,a,c;

					a1=q[3]*(255-Alpha);
					a2=p[3]*Alpha;
					a=a1+a2;
					c=GRAIN_MERGE(q[0],p[0]);
					q[0]=COMPOSITE(q[0],a1,c,a2,a);
					c=GRAIN_MERGE(q[1],p[1]);
					q[1]=COMPOSITE(q[1],a1,c,a2,a);
					c=GRAIN_MERGE(q[2],p[2]);
					q[2]=COMPOSITE(q[2],a1,c,a2,a);
				}
				p+=4;
				q+=4;
			}
		}
		break;
	}
}


static xcflError IndexedComposite(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflUInt SrcBytesPerPixel,
	xcflUInt Opacity,xcflSize Length)
{
	xcflSize i;
	const xcflUInt8 *p;
	xcflUInt8 *q;

	if (Opacity<128)
		return XCFL_ERR_SUCCESS;

	p=(const xcflUInt8*)pSrcData;
	q=(xcflUInt8*)pDstData;

	switch (DstBytesPerPixel) {
	case 1:
		if (SrcBytesPerPixel==1) {
			xcflMemoryCopy(q,p,Length);
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				if (p[1]*Opacity>255*255/2)
					*q=p[0];
				p+=2;
				q++;
			}
		}
		break;

	case 2:
		if (SrcBytesPerPixel==1) {
			for (i=0;i<Length;i++) {
				*q++=*p++;
				*q++=255;
			}
		} else if (SrcBytesPerPixel==2) {
			for (i=0;i<Length;i++) {
				if (p[1]*Opacity>255*255/2) {
					q[0]=p[0];
					q[1]=255;
				}
				p+=2;
				q+=2;
			}
		}
		break;

	default:
		return XCFL_ERR_INVALID_ARGUMENT;
	}

	return XCFL_ERR_SUCCESS;
}


XCFL_EXPORT(xcflError) xcflCompositePixels(
	void *pDstData,xcflUInt DstBytesPerPixel,
	const void *pSrcData,xcflImageType ImageType,
	xcflCompositeMode Composite,xcflUInt Opacity,xcflSize Length)
{
	xcflUInt SrcBytesPerPixel;

	if (pDstData==NULL || pSrcData==NULL || Opacity>255 || Length==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	SrcBytesPerPixel=xcflGetImageTypePixelBytes(ImageType);
	if (SrcBytesPerPixel==0)
		return XCFL_ERR_INVALID_ARGUMENT;

	if (Opacity==0)
		return XCFL_ERR_SUCCESS;

	if (ImageType==XCFL_IMAGE_TYPE_INDEXED
			|| ImageType==XCFL_IMAGE_TYPE_INDEXED_ALPHA) {
		return IndexedComposite(pDstData,DstBytesPerPixel,
								pSrcData,SrcBytesPerPixel,
								Opacity,Length);
	}

	switch (Composite) {
	case XCFL_COMPOSITE_NORMAL:
		NormalComposite(pDstData,DstBytesPerPixel,
						pSrcData,SrcBytesPerPixel,
						Opacity,Length);
		break;

	case XCFL_COMPOSITE_DISSOLVE:
		/* TODO: Support this mode. */
		break;

	case XCFL_COMPOSITE_BEHIND:
		break;

	case XCFL_COMPOSITE_MULTIPLY:
		MultiplyComposite(pDstData,DstBytesPerPixel,
						  pSrcData,SrcBytesPerPixel,
						  Opacity,Length);
		break;

	case XCFL_COMPOSITE_SCREEN:
		ScreenComposite(pDstData,DstBytesPerPixel,
						pSrcData,SrcBytesPerPixel,
						Opacity,Length);
		break;

	case XCFL_COMPOSITE_OVERLAY:
		OverlayComposite(pDstData,DstBytesPerPixel,
						 pSrcData,SrcBytesPerPixel,
						 Opacity,Length);
		break;

	case XCFL_COMPOSITE_DIFFERENCE:
		DifferenceComposite(pDstData,DstBytesPerPixel,
							pSrcData,SrcBytesPerPixel,
							Opacity,Length);
		break;

	case XCFL_COMPOSITE_ADDITION:
		AdditionComposite(pDstData,DstBytesPerPixel,
						  pSrcData,SrcBytesPerPixel,
						  Opacity,Length);
		break;

	case XCFL_COMPOSITE_SUBTRACT:
		SubtractComposite(pDstData,DstBytesPerPixel,
						  pSrcData,SrcBytesPerPixel,
						  Opacity,Length);
		break;

	case XCFL_COMPOSITE_DARKEN:
		DarkenComposite(pDstData,DstBytesPerPixel,
						pSrcData,SrcBytesPerPixel,
						Opacity,Length);
		break;

	case XCFL_COMPOSITE_LIGHTEN:
		LightenComposite(pDstData,DstBytesPerPixel,
						 pSrcData,SrcBytesPerPixel,
						 Opacity,Length);
		break;

	case XCFL_COMPOSITE_HUE:
		HueComposite(pDstData,DstBytesPerPixel,
					 pSrcData,SrcBytesPerPixel,
					 Opacity,Length);
		break;

	case XCFL_COMPOSITE_SATURATION:
		SaturationComposite(pDstData,DstBytesPerPixel,
							pSrcData,SrcBytesPerPixel,
							Opacity,Length);
		break;

	case XCFL_COMPOSITE_COLOR:
		ColorComposite(pDstData,DstBytesPerPixel,
					   pSrcData,SrcBytesPerPixel,
					   Opacity,Length);
		break;

	case XCFL_COMPOSITE_VALUE:
		ValueComposite(pDstData,DstBytesPerPixel,
					   pSrcData,SrcBytesPerPixel,
					   Opacity,Length);
		break;

	case XCFL_COMPOSITE_DIVIDE:
		DivideComposite(pDstData,DstBytesPerPixel,
						pSrcData,SrcBytesPerPixel,
						Opacity,Length);
		break;

	case XCFL_COMPOSITE_DODGE:
		DodgeComposite(pDstData,DstBytesPerPixel,
					   pSrcData,SrcBytesPerPixel,
					   Opacity,Length);
		break;

	case XCFL_COMPOSITE_BURN:
		BurnComposite(pDstData,DstBytesPerPixel,
					  pSrcData,SrcBytesPerPixel,
					  Opacity,Length);
		break;

	case XCFL_COMPOSITE_HARDLIGHT:
		HardlightComposite(pDstData,DstBytesPerPixel,
						   pSrcData,SrcBytesPerPixel,
						   Opacity,Length);
		break;

	case XCFL_COMPOSITE_SOFTLIGHT:
		SoftlightComposite(pDstData,DstBytesPerPixel,
						   pSrcData,SrcBytesPerPixel,
						   Opacity,Length);
		break;

	case XCFL_COMPOSITE_GRAIN_EXTRACT:
		GrainExtractComposite(pDstData,DstBytesPerPixel,
							  pSrcData,SrcBytesPerPixel,
							  Opacity,Length);
		break;

	case XCFL_COMPOSITE_GRAIN_MERGE:
		GrainMergeComposite(pDstData,DstBytesPerPixel,
							pSrcData,SrcBytesPerPixel,
							Opacity,Length);
		break;

	case XCFL_COMPOSITE_COLOR_ERASE:
	case XCFL_COMPOSITE_ERASE:
	case XCFL_COMPOSITE_REPLACE:
	case XCFL_COMPOSITE_ANTI_ERASE:
		break;

	default:
		return XCFL_ERR_INVALID_ARGUMENT;
	}

	return XCFL_ERR_SUCCESS;
}
