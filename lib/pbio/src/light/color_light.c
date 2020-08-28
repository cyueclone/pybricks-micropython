// SPDX-License-Identifier: MIT
// Copyright (c) 2020 The Pybricks Authors

// Functions to control color lights such as RGB LEDs.

#include <pbio/config.h>

#if PBIO_CONFIG_LIGHT

#include <contiki.h>

#include <pbio/color.h>
#include <pbio/error.h>
#include <pbio/light.h>
#include <pbio/util.h>

#include "animation.h"
#include "color_light.h"

/**
 * Initializes the required fields of a color light data structure.
 * @param [in]  light       The light instance
 * @param [in]  funcs       The callbacks
 */
void pbio_color_light_init(pbio_color_light_t *light, const pbio_color_light_funcs_t *funcs) {
    light->funcs = funcs;
    pbio_light_animation_init(&light->animation, NULL);
}

/**
 * Stops animating the light. Has no effect if animation is not started.
 *
 * @param [in]  light       The light instance
 */
static void pbio_color_light_stop_animation(pbio_color_light_t *light) {
    if (pbio_light_animation_is_started(&light->animation)) {
        pbio_light_animation_stop(&light->animation);
    }
}

/**
 * Turns the light on with specified color and brightness.
 *
 * Some lights may not be capable of displaying all colors. Some lights may not
 * have adjustable brightness.
 *
 * The V in HSV will be the brightness of the LED. Colors like brown and gray
 * are not possible since lights emit light rather than reflect it.
 *
 * If an animation is running in the background, it will be stopped.
 *
 * @param [in]  light       The light instance
 * @param [in]  hsv         The color and brightness
 * @return                  ::PBIO_SUCCESS if the call was successful,
 *                          ::PBIO_ERROR_NO_DEV if the light is not connected
 */
pbio_error_t pbio_color_light_on_hsv(pbio_color_light_t *light, const pbio_color_hsv_t *hsv) {
    pbio_color_light_stop_animation(light);
    return light->funcs->set_hsv(light, hsv);
}

/**
 * Turns the light off.
 *
 * If an animation is running in the background, it will be stopped.
 *
 * @param [in]  light       The light instance
 * @return                  ::PBIO_SUCCESS if the call was successful,
 *                          ::PBIO_ERROR_INVALID_PORT if port is not a valid port
 *                          ::PBIO_ERROR_NO_DEV if port is valid but light is not connected
 */
pbio_error_t pbio_color_light_off(pbio_color_light_t *light) {
    pbio_color_hsv_t hsv = { 0 };
    return pbio_color_light_on_hsv(light, &hsv);
}

static clock_time_t pbio_color_light_blink_next(pbio_light_animation_t *animation) {
    pbio_color_light_t *light = PBIO_CONTAINER_OF(animation, pbio_color_light_t, animation);

    const pbio_color_light_blink_cell_t *cells = light->cells;
    const pbio_color_light_blink_cell_t *cell = &cells[light->current_cell++];

    // if we have reached the array terminator, start back at the beginning
    if (cell->duration == 0) {
        cell = &cells[0];
        light->current_cell = 1;
    }

    light->funcs->set_hsv(light, &cell->hsv);
    return clock_from_msec(cell->duration);
}

/**
 * Starts blinking the light.
 *
 * This will start a background timer to blink the lights using the information
 * in @p cells. The data in @p cells must remain valid until animation is stopped.
 *
 * If another animation is running in the background, it will be stopped and
 * replaced with this one.
 *
 * @param [in]  light       The light instance
 * @param [in]  cells       Array of up to 65536 blink animation cells ending with ::PBIO_COLOR_LIGHT_BLINK_END
 */
void pbio_color_light_start_blink_animation(pbio_color_light_t *light, const pbio_color_light_blink_cell_t *cells) {
    pbio_color_light_stop_animation(light);
    pbio_light_animation_init(&light->animation, pbio_color_light_blink_next);
    light->cells = cells;
    light->current_cell = 0;
    pbio_light_animation_start(&light->animation);
}

static clock_time_t pbio_color_light_animate_next(pbio_light_animation_t *animation) {
    pbio_color_light_t *light = PBIO_CONTAINER_OF(animation, pbio_color_light_t, animation);

    const pbio_color_hsv_t *cells = light->cells;
    const pbio_color_hsv_t *cell = &cells[light->current_cell++];

    // if we have reached the array terminator, start back at the beginning
    if (cell->h == UINT16_MAX) {
        cell = &cells[0];
        light->current_cell = 1;
    }

    light->funcs->set_hsv(light, cell);
    return clock_from_msec(light->interval);
}

/**
 * Starts animating the light.
 *
 * This will start a background timer to animate the lights using the information
 * in @p cells. The data in @p cells must remain valid until animation is stopped.
 *
 * If another animation is running in the background, it will be stopped and
 * replaced with this one.
 *
 * @param [in]  light       The light instance
 * @param [in]  interval    The the time intervale between animaction cells in milliseconds
 * @param [in]  cells       Array of up to 65536 animation cells ending with ::PBIO_COLOR_LIGHT_ANIMATION_END
 */
void pbio_color_light_start_animation(pbio_color_light_t *light, uint16_t interval, const pbio_color_hsv_t *cells) {
    pbio_color_light_stop_animation(light);
    pbio_light_animation_init(&light->animation, pbio_color_light_animate_next);
    light->interval = interval;
    light->cells = cells;
    light->current_cell = 0;
    pbio_light_animation_start(&light->animation);
}

#endif // PBIO_CONFIG_LIGHT
