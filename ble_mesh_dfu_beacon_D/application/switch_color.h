/**
 * @file switch_color.h
 * @brief 
 * @date Wed 16 Sep 2015 03:14:03 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

#ifndef __SWITCH_COLOR_H__
#define __SWITCH_COLOR_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
}COLOR_RGB;

typedef struct
{
    float hue;          //[0, 360]
    float saturation;   //[0, 100]
    float luminance;    //[0, 100]
}COLOR_HSL;


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
void rgb_to_hsl(const COLOR_RGB *rgb, COLOR_HSL *hsl);
void hsl_to_rgb(const COLOR_HSL *hsl, COLOR_RGB *rgb);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

