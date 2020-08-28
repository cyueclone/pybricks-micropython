// SPDX-License-Identifier: MIT
// Copyright (c) 2020 The Pybricks Authors

/**
 * @addtogroup Color Color Conversion Functions.
 *
 * Provides generic functions to manipulate colors.
 * @{
 */

#ifndef _PBIO_COLOR_H_
#define _PBIO_COLOR_H_

#include <stdint.h>

/** @cond INTERNAL */

/**
 * Packs HSV and duration into 32 bits pbio_color_t
 * 
 * @param [in]  hue         The hue (0 to 359)
 * @param [in]  saturation  The saturation (0 to 100)
 * @param [in]  value       The brightness (0 to 100)
 * @param [in]  duration    The duration in milliseconds (0--4096 ms, truncates to 4ms steps)
 */
#define PBIO_COLOR_ENCODE(hue, saturation, value, duration) \
    ( \
        ((hue)               & 0x01FF) << 0  | \
        (((saturation) >> 1) & 0x3F)   << 9  | \
        ((value)             & 0x7F)   << 15 | \
        (((duration) >> 2)   & 0x3FF)  << 22   \
    )

/**
 * Gets duration from pbio_color_t
 * 
 * @param [in]  color       Encoded using PBIO_COLOR_ENCODE
 * @param [in]  duration    The duration in milliseconds (0--4096 ms, truncates to 4ms steps)
 */
#define PBIO_COLOR_GET_DURATION(color) (((color) >> 22) << 2)

/** @endcond */

/**
 * Color names.
 */
typedef enum {
    // NONE uses different hue to differintiate if from black
    PBIO_COLOR_NONE = PBIO_COLOR_ENCODE(180, 0, 0, 0), /**< no color */
    PBIO_COLOR_BLACK = PBIO_COLOR_ENCODE(0, 0, 0, 0), /**< black */
    PBIO_COLOR_GRAY = PBIO_COLOR_ENCODE(0, 0, 50, 0), /**< gray */
    PBIO_COLOR_WHITE = PBIO_COLOR_ENCODE(0, 0, 100, 0), /**< white */
    PBIO_COLOR_RED = PBIO_COLOR_ENCODE(0, 100, 100, 0), /**< red */
    PBIO_COLOR_BROWN = PBIO_COLOR_ENCODE(30, 100, 50, 0), /**< brown */
    PBIO_COLOR_ORANGE = PBIO_COLOR_ENCODE(30, 100, 100, 0), /**< orange */
    PBIO_COLOR_YELLOW = PBIO_COLOR_ENCODE(60, 100, 100, 0), /**< yellow */
    PBIO_COLOR_GREEN = PBIO_COLOR_ENCODE(120, 100, 100, 0), /**< green */
    PBIO_COLOR_CYAN = PBIO_COLOR_ENCODE(180, 100, 100, 0), /**< cyan */
    PBIO_COLOR_BLUE = PBIO_COLOR_ENCODE(240, 100, 100, 0), /**< blue */
    PBIO_COLOR_VIOLET = PBIO_COLOR_ENCODE(270, 100, 100, 0), /**< violet */
    PBIO_COLOR_MAGENTA = PBIO_COLOR_ENCODE(300, 100, 100, 0), /**< magenta */
} pbio_color_t;

#undef PBIO_COLOR_ENCODE

/** 24-bit RGB color. */
typedef struct {
    /** The red component. 0 to 255. */
    uint8_t r;
    /** The green component. 0 to 255. */
    uint8_t g;
    /** The blue component. 0 to 255. */
    uint8_t b;
} pbio_color_rgb_t;

/** HSV color. */
typedef struct {
    /** The hue component. 0 to 359 degrees. */
    uint16_t h;
    /** The saturation component. 0 to 100 percent. */
    uint8_t s;
    /** The value component. 0 to 100 percent. */
    uint8_t v;
} pbio_color_hsv_t;


void pbio_color_rgb_to_hsv(const pbio_color_rgb_t *rgb, pbio_color_hsv_t *hsv);
void pbio_color_hsv_to_rgb(const pbio_color_hsv_t *hsv, pbio_color_rgb_t *rgb);
void pbio_color_to_hsv(pbio_color_t color, pbio_color_hsv_t *hsv);
void pbio_color_to_rgb(pbio_color_t color, pbio_color_rgb_t *rgb);

#endif // _PBIO_COLOR_H_

/**
 * @}
 */
