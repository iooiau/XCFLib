/******************************************************************************
*                                                                             *
*    xcfl_colorspace.h                      Copyright(c) 2010-2013 itow,y.    *
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
#include "xcfl_colorspace.h"
#include <math.h>


#define XCFL_COLORSPACE_HSV_USE_INT

#define xcflRound(v)	((int)((v)+0.5))
#define xcflScale8(v)	xcflRound((v)*255.0)




XCFL_EXPORT(void) xcflRGBToHSVInt(int *pRed,int *pGreen,int *pBlue)
{
#ifndef XCFL_COLORSPACE_HSV_USE_INT

	double r,g,b;
	double h,s,v;
	double Min,Delta;

	r=*pRed;
	g=*pGreen;
	b=*pBlue;

	if (r>g) {
		v=xcflMax(r,b);
		Min=xcflMin(g,b);
	} else {
		v=xcflMax(g,b);
		Min=xcflMin(r,b);
	}

	Delta=v-Min;

	if (v==0.0)
		s=0.0;
	else
		s=Delta/v;

	if (s==0.0) {
		h=0.0;
	} else {
		if (r==v)
			h=60.0*(g-b)/Delta;
		else if (g==v)
			h=120.0+60.0*(b-r)/Delta;
		else
			h=240.0+60.0*(r-g)/Delta;

		if (h<0.0)
			h+=360.0;
		else if (h>360.0)
			h-=360.0;
	}

	*pRed=xcflRound(h);
	*pGreen=xcflScale8(s);
	*pBlue=xcflRound(v);

#else	/* XCFL_COLORSPACE_HSV_USE_INT */

	int r,g,b;
	int h,s,v;
	int Min,Delta;

	r=*pRed;
	g=*pGreen;
	b=*pBlue;

	if (r>g) {
		v=xcflMax(r,b);
		Min=xcflMin(g,b);
	} else {
		v=xcflMax(g,b);
		Min=xcflMin(r,b);
	}

	Delta=v-Min;

	if (v==0)
		s=0;
	else
		s=Delta*255/v;

	if (s==0) {
		h=0;
	} else {
		if (r==v)
			h=60*(g-b)/Delta;
		else if (g==v)
			h=120+60*(b-r)/Delta;
		else
			h=240+60*(r-g)/Delta;

		if (h<0)
			h+=360;
		else if (h>360)
			h-=360;
	}

	*pRed=h;
	*pGreen=s;
	*pBlue=v;

#endif	/* XCFL_COLORSPACE_HSV_USE_INT */
}


XCFL_EXPORT(void) xcflHSVToRGBInt(int *pHue,int *pSaturation,int *pValue)
{
	if (*pSaturation==0) {
		*pHue=*pValue;
		*pSaturation=*pValue;
	} else {
		double h,s,v,h_temp;
		double f,p,q,t;
		int i;

		h=*pHue;
		s=(double)*pSaturation/255.0;
		v=(double)*pValue/255.0;

		if (h==360)
			h_temp=0;
		else
			h_temp=h;

		h_temp/=60.0;
		i=(int)floor(h_temp);
		f=h_temp-i;
		p=v*(1.0-s);
		q=v*(1.0-(s*f));
		t=v*(1.0-(s*(1.0-f)));

		switch (i) {
		case 0:
			*pHue=xcflScale8(v);
			*pSaturation=xcflScale8(t);
			*pValue=xcflScale8(p);
			break;

		case 1:
			*pHue=xcflScale8(q);
			*pSaturation=xcflScale8(v);
			*pValue=xcflScale8(p);
			break;

		case 2:
			*pHue=xcflScale8(p);
			*pSaturation=xcflScale8(v);
			*pValue=xcflScale8(t);
			break;

		case 3:
			*pHue=xcflScale8(p);
			*pSaturation=xcflScale8(q);
			*pValue=xcflScale8(v);
			break;

		case 4:
			*pHue=xcflScale8(t);
			*pSaturation=xcflScale8(p);
			*pValue=xcflScale8(v);
			break;

		case 5:
			*pHue=xcflScale8(v);
			*pSaturation=xcflScale8(p);
			*pValue=xcflScale8(q);
			break;
		}
	}
}


XCFL_EXPORT(int) xcflRGBToHSVInt_S(int Red,int Green,int Blue)
{
	int Max,Min;

	if (Red>Green) {
		Max=xcflMax(Red,Blue);
		Min=xcflMin(Green,Blue);
	} else {
		Max=xcflMax(Green,Blue);
		Min=xcflMin(Red,Blue);
	}

	if (Max==0)
		return 0;
	return ((Max-Min)*255)/Max;
}


XCFL_EXPORT(void) xcflRGBToHSLInt(int *pRed,int *pGreen,int *pBlue)
{
	int r,g,b;
	double h,s,l;
	int Min,Max,Delta;

	r=*pRed;
	g=*pGreen;
	b=*pBlue;

	if (r>g) {
		Max=xcflMax(r,b);
		Min=xcflMin(g,b);
	} else {
		Max=xcflMax(g,b);
		Min=xcflMin(r,b);
	}

	l=(Max+Min)/2.0;

	if (Max==Min) {
		*pRed=0;
		*pGreen=0;
	} else {
		Delta=Max-Min;

		if (l<128.0)
			s=255.0*(double)Delta/(double)(Max+Min);
		else
			s=255.0*(double)Delta/(double)(511-Max-Min);

		if (r==Max)
			h=(double)(g-b)/(double)Delta;
		else if (g==Max)
			h=2.0+(double)(b-r)/(double)Delta;
		else
			h=4.0+(double)(r-g)/(double)Delta;

		h*=42.5;

		if (h<0.0)
			h+=255.0;
		else if (h>255.0)
			h-=255.0;

		*pRed=xcflRound(h);
		*pGreen=xcflRound(s);
	}

	*pBlue=xcflRound(l);
}


static int HSLValueInt(double n1,double n2,double Hue)
{
	double Value;

	if (Hue>255.0)
		Hue-=255.0;
	else if (Hue<0.0)
		Hue+=255.0;

	if (Hue<42.5)
		Value=n1+(n2-n1)*(Hue/42.5);
	else if (Hue<127.5)
		Value=n2;
	else if (Hue<170.0)
		Value=n1+(n2-n1)*((170.0-Hue)/42.5);
	else
		Value=n1;

	return xcflScale8(Value);
}

XCFL_EXPORT(void) xcflHSLToRGBInt(int *pHue,int *pSaturation,int *pLightness)
{
	if (*pSaturation==0) {
		*pHue=*pLightness;
		*pSaturation=*pLightness;
	} else {
		double h,s,l;
		double m1, m2;

		h=*pHue;
		s=*pSaturation;
		l=*pLightness;

		if (l<128.0)
			m2=(l*(255.0+s))/65025.0;
		else
			m2=(l+s-(l*s)/255.0)/255.0;

		m1=(l/127.5)-m2;

		*pHue=HSLValueInt(m1,m2,h+85.0);
		*pSaturation=HSLValueInt(m1,m2,h);
		*pLightness=HSLValueInt(m1,m2,h-85.0);
	}
}
