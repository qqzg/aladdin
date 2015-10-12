#include <math.h>
#include "changecolor.h"
#define PI (3.14)
static int mColor;


/**
     * Return a color-int from alpha, red, green, blue components.
     * These component values should be [0..255], but there is no
     * range check performed, so if they are out of range, the
     * returned color is undefined.
     * @param alpha Alpha component [0..255] of the color
     * @param red   Red component [0..255] of the color
     * @param green Green component [0..255] of the color
     * @param blue  Blue component [0..255] of the color
     */
static int argb(int alpha, int red, int green, int blue) {
	return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

static int COLORS[] = { 0xFFFF0000, 0xFFFF00FF,
  0xFF0000FF, 0xFF00FFFF, 0xFF00FF00, 0xFFFFFF00, 0xFFFF0000 };

int ave(int s, int d, float p) {
   return s + round(p * (d - s));
}

/**
 * Return the alpha component of a color int. This is the same as saying
 * color >>> 24
 */
static int alpha(int color) {
    return color >> 24;
}

/**
 * Return the red component of a color int. This is the same as saying
 * (color >> 16) & 0xFF
 */
static int red(int color) {
    return (color >> 16) & 0xFF;
}

/**
 * Return the green component of a color int. This is the same as saying
 * (color >> 8) & 0xFF
 */
static int green(int color) {
    return (color >> 8) & 0xFF;
}

/**
 * Return the blue component of a color int. This is the same as saying
 * color & 0xFF
 */
static int blue(int color) {
    return color & 0xFF;
}

/**
 * Return a color-int from red, green, blue components.
 * The alpha component is implicity 255 (fully opaque).
 * These component values should be [0..255], but there is no
 * range check performed, so if they are out of range, the
 * returned color is undefined.
 * @param red  Red component [0..255] of the color
 * @param green Green component [0..255] of the color
 * @param blue  Blue component [0..255] of the color
 */
static int rgb(int red, int green, int blue) {
    return ((int)0xFF << 24) | (red << 16) | (green << 8) | blue;
}



/**
 * Calculate the color using the supplied angle.
 *
 * @param angle The selected color's position expressed as angle (in rad).
 *
 * @return The ARGB value of the color on the color wheel at the specified
 *         angle.
 */
int calculateColor(float angle) {
   float unit = (float) (angle / (2 * PI));
   if (unit < 0) {
      unit += 1;
   }

   if (unit <= 0) {
      mColor = COLORS[0];
      return COLORS[0];
   }
   if (unit >= 1) {
      mColor = COLORS[sizeof(COLORS)/sizeof(COLORS[0]) - 1];
      return COLORS[sizeof(COLORS)/sizeof(COLORS[0]) - 1];
   }

   float p = unit * (sizeof(COLORS)/sizeof(COLORS[0]) - 1);
   int i = (int) p;
   p -= i;

   int c0 = COLORS[i];
   int c1 = COLORS[i + 1];
   int a = ave(alpha(c0), alpha(c1), p);
   int r = ave(red(c0), red(c1), p);
   int g = ave(green(c0), green(c1), p);
   int b = ave(blue(c0), blue(c1), p);

   mColor = argb(a, r, g, b);
   return argb(a, r, g, b);
}