/**
 * @file switch_color.c
 * @brief 
 * @date Wed 16 Sep 2015 03:13:30 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */

#include <string.h>
#include "switch_color.h"
/*********************************************************************
 * MACROS
 */

#define min3v(v1, v2, v3)   ((v1)>(v2)? ((v2)>(v3)?(v3):(v2)):((v1)>(v3)?(v3):(v2)))
#define max3v(v1, v2, v3)   ((v1)<(v2)? ((v2)<(v3)?(v3):(v2)):((v1)<(v3)?(v3):(v1)))
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

void rgb_to_hsl(const COLOR_RGB *rgb, COLOR_HSL *hsl)
{
	float h = 0, s = 0, l = 0;
    // normalizes red-green-blue values
    float r = ((float)rgb->red)   / 255.0;
    float g = ((float)rgb->green) / 255.0;
    float b = ((float)rgb->blue)  / 255.0;
    float maxVal = max3v(r, g, b);
    float minVal = min3v(r, g, b);
	// hue
	if(maxVal == minVal)
	{
		h = 0; // undefined
	}
	else if(maxVal==r && g>=b)
	{
        h = 60.0f*(g-b)/(maxVal-minVal);
	}
    else if(maxVal==r && g<b)
	{
		h = 60.0f*(g-b)/(maxVal-minVal) + 360.0f;
	}
	else if(maxVal==g)
	{
		h = 60.0f*(b-r)/(maxVal-minVal) + 120.0f;
	}
	else if(maxVal==b)
	{
		h = 60.0f*(r-g)/(maxVal-minVal) + 240.0f;
	}
    // luminance
	l = (maxVal+minVal)/2.0f;
	// saturation
	if(l == 0 || maxVal == minVal)
	{
		s = 0;
	}
	else if(0<l && l<=0.5f)
	{
		s = (maxVal-minVal)/(maxVal+minVal);
	}
	else if(l>0.5f)
	{
		s = (maxVal-minVal)/(2 - (maxVal+minVal)); //(maxVal-minVal > 0)?
	}
	hsl->hue = (h>360)? 360 : ((h<0)?0:h); 
	hsl->saturation = ((s>1)? 1 : ((s<0)?0:s))*100;
	hsl->luminance = ((l>1)? 1 : ((l<0)?0:l))*100;
}

void hsl_to_rgb(const COLOR_HSL *hsl, COLOR_RGB *rgb) 
{
	float h = hsl->hue;				    // h must be [0, 360]
	float s = hsl->saturation/100.f;	// s must be [0, 1]
	float l = hsl->luminance/100.f;	    // l must be [0, 1]
	float R, G, B;
    int i = 0;
	if(hsl->saturation == 0)
	{
		// achromatic color (gray scale)
		R = G = B = l*255.f;
	}
	else
	{
	float q = (l<0.5f)?(l * (1.0f+s)):(l+s - (l*s));
	float p = (2.0f * l) - q;
	float Hk = h/360.0f;
	float T[3];
    T[0] = Hk + 0.3333333f;		// Tr¡¡0.3333333f=1.0/3.0
    T[1] = Hk;					// Tb
    T[2] = Hk - 0.3333333f;		// Tg
    for(i=0; i<3; i++)
    {
		if(T[i] < 0) T[i] += 1.0f;
		if(T[i] > 1) T[i] -= 1.0f;
		if((T[i]*6) < 1)
		{
			T[i] = p + ((q-p)*6.0f*T[i]);
		}
		else if((T[i]*2.0f) < 1) //(1.0/6.0)<=T[i] && T[i]<0.5
		{
			T[i] = q;
		}
		else if((T[i]*3.0f) < 2) // 0.5<=T[i] && T[i]<(2.0/3.0)
		{
			T[i] = p + (q-p) * ((2.0f/3.0f) - T[i]) * 6.0f;
		}
		else T[i] = p;
	}
	R = T[0]*255.0f;
	G = T[1]*255.0f;
	B = T[2]*255.0f;
	}
	rgb->red = (unsigned char)((R > 255)? 255 : ((R < 0)?0 : R));
	rgb->green = (unsigned char)((G > 255)? 255 : ((G < 0)?0 : G));
	rgb->blue = (unsigned char)((B > 255)? 255 : ((B < 0)?0 : B));
}
/** @} */


