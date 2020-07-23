// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2020 The Pybricks Authors

#include <pbio/control.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mpconfig.h"
#include "py/mphal.h"

#include "modbuiltins.h"
#include "modparameters.h"

#include "pbdevice.h"
#include "pberror.h"
#include "pbobj.h"
#include "pbkwarg.h"

// pybricks.builtins.Light class object
typedef struct _builtins_ColorLight_obj_t {
    mp_obj_base_t base;
    pbdevice_t *pbdev;
} builtins_ColorLight_obj_t;

// pybricks.builtins.ColorLight.on
STATIC mp_obj_t builtins_ColorLight_on(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Parse arguments
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        builtins_ColorLight_obj_t, self,
        PB_ARG_REQUIRED(color),
        PB_ARG_DEFAULT_INT(brightness, 100));

    if (color == mp_const_none) {
        color = pb_const_color_black;
    }

    pbio_light_color_t color_id = pb_type_enum_get_value(color, &pb_enum_type_Color);

    mp_int_t bright = pb_obj_get_int(brightness);
    bright = bright < 0 ? 0 : bright > 100 ? 100 : bright;

    // TODO: Brightness control is not yet implemented
    if (bright != 100) {
        pb_assert(PBIO_ERROR_NOT_IMPLEMENTED);
    }

    if (!self->pbdev) {
        // No external device, so assume command is for the internal light
        pb_assert(pbio_light_on(PBIO_PORT_SELF, color_id));
    } else {
        pbdevice_color_light_on(self->pbdev, color_id);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_ColorLight_on_obj, 1, builtins_ColorLight_on);

// pybricks.builtins.ColorLight.off
STATIC mp_obj_t builtins_ColorLight_off(mp_obj_t self_in) {
    builtins_ColorLight_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Turn the light off, using the command specific to the device.
    if (!self->pbdev) {
        // No external device, so assume command is for the internal light
        pb_assert(pbio_light_off(PBIO_PORT_SELF));
    } else {
        pbdevice_color_light_on(self->pbdev, PBIO_LIGHT_COLOR_NONE);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(builtins_ColorLight_off_obj, builtins_ColorLight_off);

// dir(pybricks.builtins.ColorLight)
STATIC const mp_rom_map_elem_t builtins_ColorLight_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&builtins_ColorLight_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&builtins_ColorLight_off_obj) },
};
STATIC MP_DEFINE_CONST_DICT(builtins_ColorLight_locals_dict, builtins_ColorLight_locals_dict_table);

// type(pybricks.builtins.ColorLight)
const mp_obj_type_t builtins_ColorLight_type = {
    { &mp_type_type },
    .name = MP_QSTR_ColorLight,
    .locals_dict = (mp_obj_dict_t *)&builtins_ColorLight_locals_dict,
};

mp_obj_t builtins_ColorLight_obj_make_new(pbdevice_t *pbdev) {
    // Create new light instance
    builtins_ColorLight_obj_t *light = m_new_obj(builtins_ColorLight_obj_t);
    // Set type and iodev
    light->base.type = &builtins_ColorLight_type;
    light->pbdev = pbdev;
    return light;
}

// pybricks.builtins.Light class object
typedef struct _builtins_LightArray_obj_t {
    mp_obj_base_t base;
    pbdevice_t *pbdev;
    uint8_t light_mode;
    uint8_t number_of_lights;
} builtins_LightArray_obj_t;

// pybricks.builtins.LightArray.on
STATIC mp_obj_t builtins_LightArray_on(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Parse arguments
    builtins_LightArray_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // We may either have one brightness or all brightness values
    if (!(n_args == 2 || n_args == (size_t)self->number_of_lights + 1)) {
        pb_assert(PBIO_ERROR_INVALID_ARG);
    }

    // Get brightness values from arguments
    int32_t brightness[4];
    for (uint8_t i = 0; i < self->number_of_lights; i++) {

        // If only one brightness was given, apply it to all
        if (n_args == 2 && i > 0) {
            brightness[i] = brightness[0];
            continue;
        }

        // Get brightness from arguments
        brightness[i] = pb_obj_get_int(pos_args[i + 1]);
        brightness[i] = brightness[i] < 0 ? 0 : brightness[i] > 100 ? 100 : brightness[i];
    }

    // Set the brightness values
    pbdevice_set_values(self->pbdev, self->light_mode, brightness, self->number_of_lights);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_LightArray_on_obj, 1, builtins_LightArray_on);

// pybricks.builtins.LightArray.off
STATIC mp_obj_t builtins_LightArray_off(mp_obj_t self_in) {
    builtins_LightArray_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int32_t brightness[4] = {0};
    pbdevice_set_values(self->pbdev, self->light_mode, brightness, self->number_of_lights);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(builtins_LightArray_off_obj, builtins_LightArray_off);

// dir(pybricks.builtins.LightArray)
STATIC const mp_rom_map_elem_t builtins_LightArray_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&builtins_LightArray_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&builtins_LightArray_off_obj) },
};
STATIC MP_DEFINE_CONST_DICT(builtins_LightArray_locals_dict, builtins_LightArray_locals_dict_table);

// type(pybricks.builtins.LightArray)
const mp_obj_type_t builtins_LightArray_type = {
    { &mp_type_type },
    .name = MP_QSTR_LightArray,
    .locals_dict = (mp_obj_dict_t *)&builtins_LightArray_locals_dict,
};

mp_obj_t builtins_LightArray_obj_make_new(pbdevice_t *pbdev, uint8_t light_mode, uint8_t number_of_lights) {
    // Create new light instance
    builtins_LightArray_obj_t *light = m_new_obj(builtins_LightArray_obj_t);
    // Set type and iodev
    light->base.type = &builtins_LightArray_type;
    light->pbdev = pbdev;
    light->light_mode = light_mode;
    light->number_of_lights = number_of_lights;
    return light;
}

// pybricks.builtins.Control class object structure
typedef struct _builtins_Control_obj_t {
    mp_obj_base_t base;
    pbio_control_t *control;
    mp_obj_t scale;
} builtins_Control_obj_t;

// pybricks.builtins.Control.__init__/__new__
mp_obj_t builtins_Control_obj_make_new(pbio_control_t *control) {

    builtins_Control_obj_t *self = m_new_obj(builtins_Control_obj_t);
    self->base.type = &builtins_Control_type;

    self->control = control;

    #if MICROPY_PY_BUILTINS_FLOAT
    self->scale = mp_obj_new_float(fix16_to_float(control->settings.counts_per_unit));
    #else
    self->scale = mp_obj_new_int(fix16_to_int(control->settings.counts_per_unit));
    #endif

    return self;
}

STATIC void raise_if_control_busy(pbio_control_t *ctl) {
    if (ctl->type != PBIO_CONTROL_NONE) {
        pb_assert(PBIO_ERROR_INVALID_OP);
    }
}

// pybricks.builtins.Control.limits
STATIC mp_obj_t builtins_Control_limits(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        builtins_Control_obj_t, self,
        PB_ARG_DEFAULT_NONE(speed),
        PB_ARG_DEFAULT_NONE(acceleration),
        PB_ARG_DEFAULT_NONE(actuation));

    // Read current values
    int32_t _speed, _acceleration, _actuation;
    pbio_control_settings_get_limits(&self->control->settings, &_speed, &_acceleration, &_actuation);

    // If all given values are none, return current values
    if (speed == mp_const_none && acceleration == mp_const_none && actuation == mp_const_none) {
        mp_obj_t ret[3];
        ret[0] = mp_obj_new_int(_speed);
        ret[1] = mp_obj_new_int(_acceleration);
        ret[2] = mp_obj_new_int(_actuation);
        return mp_obj_new_tuple(3, ret);
    }

    // Assert control is not active
    raise_if_control_busy(self->control);

    // Set user settings
    _speed = pb_obj_get_default_int(speed, _speed);
    _acceleration = pb_obj_get_default_int(acceleration, _acceleration);
    _actuation = pb_obj_get_default_int(actuation, _actuation);

    pb_assert(pbio_control_settings_set_limits(&self->control->settings, _speed, _acceleration, _actuation));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_Control_limits_obj, 1, builtins_Control_limits);

// pybricks.builtins.Control.pid
STATIC mp_obj_t builtins_Control_pid(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        builtins_Control_obj_t, self,
        PB_ARG_DEFAULT_NONE(kp),
        PB_ARG_DEFAULT_NONE(ki),
        PB_ARG_DEFAULT_NONE(kd),
        PB_ARG_DEFAULT_NONE(integral_range),
        PB_ARG_DEFAULT_NONE(integral_rate),
        PB_ARG_DEFAULT_NONE(feed_forward));

    // Read current values
    int16_t _kp, _ki, _kd;
    int32_t _integral_range, _integral_rate, _feed_forward;
    pbio_control_settings_get_pid(&self->control->settings, &_kp, &_ki, &_kd, &_integral_range, &_integral_rate, &_feed_forward);

    // If all given values are none, return current values
    if (kp == mp_const_none && ki == mp_const_none && kd == mp_const_none &&
        integral_range == mp_const_none && integral_rate == mp_const_none && feed_forward == mp_const_none) {
        mp_obj_t ret[6];
        ret[0] = mp_obj_new_int(_kp);
        ret[1] = mp_obj_new_int(_ki);
        ret[2] = mp_obj_new_int(_kd);
        ret[3] = mp_obj_new_int(_integral_range);
        ret[4] = mp_obj_new_int(_integral_rate);
        ret[5] = mp_obj_new_int(_feed_forward);
        return mp_obj_new_tuple(6, ret);
    }

    // Assert control is not active
    raise_if_control_busy(self->control);

    // Set user settings
    _kp = pb_obj_get_default_int(kp, _kp);
    _ki = pb_obj_get_default_int(ki, _ki);
    _kd = pb_obj_get_default_int(kd, _kd);
    _integral_range = pb_obj_get_default_int(integral_range, _integral_range);
    _integral_rate = pb_obj_get_default_int(integral_rate, _integral_rate);
    _feed_forward = pb_obj_get_default_int(feed_forward, _feed_forward);

    pb_assert(pbio_control_settings_set_pid(&self->control->settings, _kp, _ki, _kd, _integral_range, _integral_rate, _feed_forward));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_Control_pid_obj, 1, builtins_Control_pid);

// pybricks.builtins.Control.target_tolerances
STATIC mp_obj_t builtins_Control_target_tolerances(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        builtins_Control_obj_t, self,
        PB_ARG_DEFAULT_NONE(speed),
        PB_ARG_DEFAULT_NONE(position));

    // Read current values
    int32_t _speed, _position;
    pbio_control_settings_get_target_tolerances(&self->control->settings, &_speed, &_position);

    // If all given values are none, return current values
    if (speed == mp_const_none && position == mp_const_none) {
        mp_obj_t ret[2];
        ret[0] = mp_obj_new_int(_speed);
        ret[1] = mp_obj_new_int(_position);
        return mp_obj_new_tuple(2, ret);
    }

    // Assert control is not active
    raise_if_control_busy(self->control);

    // Set user settings
    _speed = pb_obj_get_default_int(speed, _speed);
    _position = pb_obj_get_default_int(position, _position);

    pb_assert(pbio_control_settings_set_target_tolerances(&self->control->settings, _speed, _position));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_Control_target_tolerances_obj, 1, builtins_Control_target_tolerances);

// pybricks.builtins.Control.stall_tolerances
STATIC mp_obj_t builtins_Control_stall_tolerances(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        builtins_Control_obj_t, self,
        PB_ARG_DEFAULT_NONE(speed),
        PB_ARG_DEFAULT_NONE(time));

    // Read current values
    int32_t _speed, _time;
    pbio_control_settings_get_stall_tolerances(&self->control->settings, &_speed, &_time);

    // If all given values are none, return current values
    if (speed == mp_const_none && time == mp_const_none) {
        mp_obj_t ret[2];
        ret[0] = mp_obj_new_int(_speed);
        ret[1] = mp_obj_new_int(_time);
        return mp_obj_new_tuple(2, ret);
    }

    // Assert control is not active
    raise_if_control_busy(self->control);

    // Set user settings
    _speed = pb_obj_get_default_int(speed, _speed);
    _time = pb_obj_get_default_int(time, _time);

    pb_assert(pbio_control_settings_set_stall_tolerances(&self->control->settings, _speed, _time));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(builtins_Control_stall_tolerances_obj, 1, builtins_Control_stall_tolerances);

// pybricks.builtins.Control.trajectory
STATIC mp_obj_t builtins_Control_trajectory(mp_obj_t self_in) {
    builtins_Control_obj_t *self = MP_OBJ_TO_PTR(self_in);
    pbio_trajectory_t trajectory;

    mp_obj_t parms[12];

    trajectory = self->control->trajectory;

    if (self->control->type != PBIO_CONTROL_NONE) {
        parms[0] = mp_obj_new_int((trajectory.t0 - trajectory.t0) / 1000);
        parms[1] = mp_obj_new_int((trajectory.t1 - trajectory.t0) / 1000);
        parms[2] = mp_obj_new_int((trajectory.t2 - trajectory.t0) / 1000);
        parms[3] = mp_obj_new_int((trajectory.t3 - trajectory.t0) / 1000);
        parms[4] = mp_obj_new_int(trajectory.th0);
        parms[5] = mp_obj_new_int(trajectory.th1);
        parms[6] = mp_obj_new_int(trajectory.th2);
        parms[7] = mp_obj_new_int(trajectory.th3);
        parms[8] = mp_obj_new_int(trajectory.w0);
        parms[9] = mp_obj_new_int(trajectory.w1);
        parms[10] = mp_obj_new_int(trajectory.a0);
        parms[11] = mp_obj_new_int(trajectory.a2);
        return mp_obj_new_tuple(12, parms);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(builtins_Control_trajectory_obj, builtins_Control_trajectory);

// pybricks.builtins.Control.done
STATIC mp_obj_t builtins_Control_done(mp_obj_t self_in) {
    builtins_Control_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(pbio_control_is_done(self->control));
}
MP_DEFINE_CONST_FUN_OBJ_1(builtins_Control_done_obj, builtins_Control_done);

// pybricks.builtins.Control.stalled
STATIC mp_obj_t builtins_Control_stalled(mp_obj_t self_in) {
    builtins_Control_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(pbio_control_is_stalled(self->control));
}
MP_DEFINE_CONST_FUN_OBJ_1(builtins_Control_stalled_obj, builtins_Control_stalled);

// dir(pybricks.builtins.Control)
STATIC const mp_rom_map_elem_t builtins_Control_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_limits), MP_ROM_PTR(&builtins_Control_limits_obj) },
    { MP_ROM_QSTR(MP_QSTR_pid), MP_ROM_PTR(&builtins_Control_pid_obj) },
    { MP_ROM_QSTR(MP_QSTR_target_tolerances), MP_ROM_PTR(&builtins_Control_target_tolerances_obj) },
    { MP_ROM_QSTR(MP_QSTR_stall_tolerances), MP_ROM_PTR(&builtins_Control_stall_tolerances_obj) },
    { MP_ROM_QSTR(MP_QSTR_trajectory), MP_ROM_PTR(&builtins_Control_trajectory_obj) },
    { MP_ROM_QSTR(MP_QSTR_done), MP_ROM_PTR(&builtins_Control_done_obj) },
    { MP_ROM_QSTR(MP_QSTR_stalled), MP_ROM_PTR(&builtins_Control_stalled_obj) },
    { MP_ROM_QSTR(MP_QSTR_scale), MP_ROM_ATTRIBUTE_OFFSET(builtins_Control_obj_t, scale) },
};
STATIC MP_DEFINE_CONST_DICT(builtins_Control_locals_dict, builtins_Control_locals_dict_table);

// type(pybricks.builtins.Control)
const mp_obj_type_t builtins_Control_type = {
    { &mp_type_type },
    .name = MP_QSTR_Control,
    .attr = pb_obj_generic_and_offset_attr,
    .locals_dict = (mp_obj_dict_t *)&builtins_Control_locals_dict,
};
