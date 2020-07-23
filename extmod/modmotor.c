// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2020 The Pybricks Authors

#include "py/mpconfig.h"

#if PBDRV_CONFIG_NUM_MOTOR_CONTROLLER != 0

#include <inttypes.h>

#include <pbio/servo.h>
#include <pbio/motorpoll.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/obj.h"

#include "modbuiltins.h"
#include "modmotor.h"
#include "modlogger.h"
#include "modparameters.h"
#include "pberror.h"
#include "pbobj.h"
#include "pbkwarg.h"

// pybricks.builtins.DCMotor.__init__
STATIC mp_obj_t motor_DCMotor_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    PB_PARSE_ARGS_CLASS(n_args, n_kw, args,
        PB_ARG_REQUIRED(port),
        PB_ARG_DEFAULT_OBJ(positive_direction, pb_Direction_CLOCKWISE_obj));

    // Configure the motor with the selected arguments at pbio level
    mp_int_t port_arg = pb_type_enum_get_value(port, &pb_enum_type_Port);
    pbio_direction_t direction_arg = pb_type_enum_get_value(positive_direction, &pb_enum_type_Direction);

    // Get and initialize DC Motor
    pbio_dcmotor_t *dc;
    pbio_error_t err;
    while ((err = pbio_dcmotor_get(port_arg, &dc, direction_arg, false)) == PBIO_ERROR_AGAIN) {
        mp_hal_delay_ms(1000);
    }
    pb_assert(err);

    // On success, create and return the MicroPython object
    motor_DCMotor_obj_t *self = m_new_obj(motor_DCMotor_obj_t);
    self->base.type = (mp_obj_type_t *)type;
    self->dcmotor = dc;
    return MP_OBJ_FROM_PTR(self);
}

// pybricks.builtins.DCMotor.__repr__
// pybricks.builtins.Motor.__repr__
void motor_DCMotor_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {

    // Get the dcmotor from self, which is either Motor or DCMotor
    pbio_dcmotor_t *dcmotor = mp_obj_is_type(self_in, &motor_Motor_type) ?
        ((motor_Motor_obj_t *)MP_OBJ_TO_PTR(self_in))->srv->dcmotor :
        ((motor_DCMotor_obj_t *)MP_OBJ_TO_PTR(self_in))->dcmotor;

    mp_printf(print,
        "Motor properties:\n"
        "------------------------\n"
        "Port\t\t %c\n"
        "Positive dir.\t %s",
        dcmotor->port,
        dcmotor->direction == PBIO_DIRECTION_CLOCKWISE ? "clockwise" : "counterclockwise");
}

// pybricks.builtins.DCMotor.dc
// pybricks.builtins.Motor.dc
STATIC mp_obj_t motor_DCMotor_duty(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Parse all arguments except the first one (self)
    PB_PARSE_ARGS_METHOD_SKIP_SELF(n_args, pos_args, kw_args,
        PB_ARG_REQUIRED(duty));

    mp_int_t duty_cycle = pb_obj_get_int(duty);

    // Object type is either Motor or DCMotor
    bool is_servo = mp_obj_is_type(pos_args[0], &motor_Motor_type);

    if (is_servo) {
        motor_Motor_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
        pb_assert(pbio_servo_set_duty_cycle(self->srv, duty_cycle));
    } else {
        motor_DCMotor_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
        pb_assert(pbio_dcmotor_set_duty_cycle_usr(self->dcmotor, duty_cycle));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_DCMotor_duty_obj, 1, motor_DCMotor_duty);

// pybricks.builtins.DCMotor.stop
// pybricks.builtins.Motor.stop
STATIC mp_obj_t motor_DCMotor_stop(mp_obj_t self_in) {

    // Object type is either Motor or DCMotor
    bool is_servo = mp_obj_is_type(self_in, &motor_Motor_type);

    if (is_servo) {
        motor_Motor_obj_t *self = MP_OBJ_TO_PTR(self_in);
        pb_assert(pbio_servo_stop(self->srv, PBIO_ACTUATION_COAST));
    } else {
        motor_DCMotor_obj_t *self = MP_OBJ_TO_PTR(self_in);
        pb_assert(pbio_dcmotor_coast(self->dcmotor));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(motor_DCMotor_stop_obj, motor_DCMotor_stop);

// pybricks.builtins.DCMotor.brake
// pybricks.builtins.Motor.brake
STATIC mp_obj_t motor_DCMotor_brake(mp_obj_t self_in) {

    // Object type is either Motor or DCMotor
    bool is_servo = mp_obj_is_type(self_in, &motor_Motor_type);

    if (is_servo) {
        motor_Motor_obj_t *self = MP_OBJ_TO_PTR(self_in);
        pb_assert(pbio_servo_stop(self->srv, PBIO_ACTUATION_BRAKE));
    } else {
        motor_DCMotor_obj_t *self = MP_OBJ_TO_PTR(self_in);
        #if PYBRICKS_PY_EV3DEVICES
        // Workaround for ev3dev dc-motor not coasting on first try
        pb_assert(pbio_dcmotor_set_duty_cycle_usr(self->dcmotor, 1));
        mp_hal_delay_ms(1);
        #endif
        pb_assert(pbio_dcmotor_brake(self->dcmotor));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(motor_DCMotor_brake_obj, motor_DCMotor_brake);

// dir(pybricks.builtins.DCMotor)
STATIC const mp_rom_map_elem_t motor_DCMotor_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_dc), MP_ROM_PTR(&motor_DCMotor_duty_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&motor_DCMotor_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_brake), MP_ROM_PTR(&motor_DCMotor_brake_obj) },
};
MP_DEFINE_CONST_DICT(motor_DCMotor_locals_dict, motor_DCMotor_locals_dict_table);

// type(pybricks.builtins.DCMotor)
const mp_obj_type_t motor_DCMotor_type = {
    { &mp_type_type },
    .name = MP_QSTR_DCMotor,
    .print = motor_DCMotor_print,
    .make_new = motor_DCMotor_make_new,
    .locals_dict = (mp_obj_dict_t *)&motor_DCMotor_locals_dict,
};

/* Wait for servo maneuver to complete */

STATIC void wait_for_completion(pbio_servo_t *srv) {
    pbio_error_t err;
    while ((err = pbio_motorpoll_get_servo_status(srv)) == PBIO_ERROR_AGAIN && !pbio_control_is_done(&srv->control)) {
        mp_hal_delay_ms(5);
    }
    if (err != PBIO_ERROR_AGAIN) {
        pb_assert(err);
    }
}

// pybricks.builtins.Motor.__init__
STATIC mp_obj_t motor_Motor_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    PB_PARSE_ARGS_CLASS(n_args, n_kw, args,
        PB_ARG_REQUIRED(port),
        PB_ARG_DEFAULT_OBJ(positive_direction, pb_Direction_CLOCKWISE_obj),
        PB_ARG_DEFAULT_NONE(gears));

    // Configure the motor with the selected arguments at pbio level
    mp_int_t port_arg = pb_type_enum_get_value(port, &pb_enum_type_Port);
    pbio_direction_t direction_arg = pb_type_enum_get_value(positive_direction, &pb_enum_type_Direction);
    pbio_error_t err;
    pbio_servo_t *srv;

    // Default gear ratio
    fix16_t gear_ratio = F16C(1, 0);

    // Parse gear argument of the form [[12, 20, 36], [20, 40]] or [12, 20, 36]
    if (gears != mp_const_none) {
        // Unpack the main list
        mp_obj_t *trains, *gear_list;
        size_t n_trains, n_gears;
        mp_obj_get_array(gears, &n_trains, &trains);

        // If the first and last element is an integer, assume the user gave just one list of gears, i.e. [12, 20, 36]
        bool is_one_train = MP_OBJ_IS_SMALL_INT(trains[0]) && MP_OBJ_IS_SMALL_INT(trains[n_trains - 1]);
        // This means we don't have a list of gear trains, but just one gear train with a given number of gears
        if (is_one_train) {
            n_gears = n_trains;
            gear_list = trains;
            n_trains = 1;
        }

        // Iterate through each of the n_trains lists
        for (size_t train = 0; train < n_trains; train++) {
            // Unless we have just one list of gears, unpack the list of gears for this train
            if (!is_one_train) {
                mp_obj_get_array(trains[train], &n_gears, &gear_list);
            }
            // For this gear train, compute the ratio from the first and last gear
            fix16_t first_gear = fix16_from_int(mp_obj_get_int(gear_list[0]));
            fix16_t last_gear = fix16_from_int(mp_obj_get_int(gear_list[n_gears - 1]));
            if (first_gear < 1 || last_gear < 1) {
                pb_assert(PBIO_ERROR_INVALID_ARG);
            }
            // Include the ratio of this train in the overall gear train
            gear_ratio = fix16_div(fix16_mul(gear_ratio, last_gear), first_gear);
        }
    }

    // Get servo device, set it up, and tell the poller if we succeeded.
    pb_assert(pbio_motorpoll_get_servo(port_arg, &srv));
    while ((err = pbio_servo_setup(srv, direction_arg, gear_ratio)) == PBIO_ERROR_AGAIN) {
        mp_hal_delay_ms(1000);
    }
    pb_assert(err);
    pb_assert(pbio_motorpoll_set_servo_status(srv, PBIO_ERROR_AGAIN));

    // On success, proceed to create and return the MicroPython object
    motor_Motor_obj_t *self = m_new_obj(motor_Motor_obj_t);
    self->base.type = (mp_obj_type_t *)type;
    self->srv = srv;

    // Create an instance of the Logger class
    self->logger = logger_obj_make_new(&self->srv->log);

    // Create an instance of the Control class
    self->control = builtins_Control_obj_make_new(&self->srv->control);

    return MP_OBJ_FROM_PTR(self);
}

// pybricks.builtins.Motor.angle
STATIC mp_obj_t motor_Motor_angle(mp_obj_t self_in) {
    motor_Motor_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int32_t angle;

    pb_assert(pbio_tacho_get_angle(self->srv->tacho, &angle));

    return mp_obj_new_int(angle);
}
MP_DEFINE_CONST_FUN_OBJ_1(motor_Motor_angle_obj, motor_Motor_angle);

// pybricks.builtins.Motor.reset_angle
STATIC mp_obj_t motor_Motor_reset_angle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_DEFAULT_NONE(angle));

    // If no angle argument is given, reset to the absolute value
    bool reset_to_abs = angle == mp_const_none;

    // Otherwise reset to the given angle
    mp_int_t reset_angle = reset_to_abs ? 0 : pb_obj_get_int(angle);

    // Set the new angle
    pb_assert(pbio_servo_reset_angle(self->srv, reset_angle, reset_to_abs));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_reset_angle_obj, 1, motor_Motor_reset_angle);

// pybricks.builtins.Motor.speed
STATIC mp_obj_t motor_Motor_speed(mp_obj_t self_in) {
    motor_Motor_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int32_t speed;

    pb_assert(pbio_tacho_get_angular_rate(self->srv->tacho, &speed));

    return mp_obj_new_int(speed);
}
MP_DEFINE_CONST_FUN_OBJ_1(motor_Motor_speed_obj, motor_Motor_speed);

// pybricks.builtins.Motor.run
STATIC mp_obj_t motor_Motor_run(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(speed));

    mp_int_t speed_arg = pb_obj_get_int(speed);
    pb_assert(pbio_servo_run(self->srv, speed_arg));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_run_obj, 1, motor_Motor_run);

// pybricks.builtins.Motor.hold
STATIC mp_obj_t motor_Motor_hold(mp_obj_t self_in) {
    motor_Motor_obj_t *self = MP_OBJ_TO_PTR(self_in);
    pb_assert(pbio_servo_stop(self->srv, PBIO_ACTUATION_HOLD));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(motor_Motor_hold_obj, motor_Motor_hold);

// pybricks.builtins.Motor.run_time
STATIC mp_obj_t motor_Motor_run_time(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(time),
        PB_ARG_DEFAULT_OBJ(then, pb_Stop_HOLD_obj),
        PB_ARG_DEFAULT_TRUE(wait));

    mp_int_t speed_arg = pb_obj_get_int(speed);
    mp_int_t time_arg = pb_obj_get_int(time);

    if (time_arg < 0 || time_arg > DURATION_MAX_S * MS_PER_SECOND) {
        pb_assert(PBIO_ERROR_INVALID_ARG);
    }

    pbio_actuation_t after_stop = pb_type_enum_get_value(then, &pb_enum_type_Stop);

    // Call pbio with parsed user/default arguments
    pb_assert(pbio_servo_run_time(self->srv, speed_arg, time_arg, after_stop));

    if (mp_obj_is_true(wait)) {
        wait_for_completion(self->srv);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_run_time_obj, 1, motor_Motor_run_time);

// pybricks.builtins.Motor.run_until_stalled
STATIC mp_obj_t motor_Motor_run_until_stalled(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(speed),
        PB_ARG_DEFAULT_OBJ(then, pb_Stop_COAST_obj),
        PB_ARG_DEFAULT_NONE(duty_limit));

    mp_int_t speed_arg = pb_obj_get_int(speed);
    pbio_actuation_t after_stop = pb_type_enum_get_value(then, &pb_enum_type_Stop);

    // If duty_limit argument, given, limit actuation during this maneuver
    bool override_duty_limit = duty_limit != mp_const_none;

    int32_t _speed, _acceleration, _actuation, user_limit;

    if (override_duty_limit) {
        // Read original values so we can restore them when we're done
        pbio_control_settings_get_limits(&self->srv->control.settings, &_speed, &_acceleration, &_actuation);

        // Get user given limit
        user_limit = pb_obj_get_int(duty_limit);
        user_limit = user_limit < 0 ? -user_limit : user_limit;
        user_limit = user_limit > 100 ? 100 : user_limit;

        // Apply the user limit
        pb_assert(pbio_control_settings_set_limits(&self->srv->control.settings, _speed, _acceleration, user_limit));
    }

    mp_obj_t ex = MP_OBJ_NULL;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // Call pbio with parsed user/default arguments
        pb_assert(pbio_servo_run_until_stalled(self->srv, speed_arg, after_stop));

        // In this command we always wait for completion, so we can return the
        // final angle below.
        wait_for_completion(self->srv);

        nlr_pop();
    } else {
        ex = MP_OBJ_FROM_PTR(nlr.ret_val);
    }

    // Restore original settings
    if (override_duty_limit) {
        pb_assert(pbio_control_settings_set_limits(&self->srv->control.settings, _speed, _acceleration, _actuation));
    }

    if (ex != MP_OBJ_NULL) {
        nlr_raise(ex);
    }

    // Read the angle upon completion of the stall maneuver
    int32_t stall_point;
    pb_assert(pbio_tacho_get_angle(self->srv->tacho, &stall_point));

    // Return angle at which the motor stalled
    return mp_obj_new_int(stall_point);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_run_until_stalled_obj, 1, motor_Motor_run_until_stalled);

// pybricks.builtins.Motor.run_angle
STATIC mp_obj_t motor_Motor_run_angle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(rotation_angle),
        PB_ARG_DEFAULT_OBJ(then, pb_Stop_HOLD_obj),
        PB_ARG_DEFAULT_TRUE(wait));

    mp_int_t speed_arg = pb_obj_get_int(speed);
    mp_int_t angle_arg = pb_obj_get_int(rotation_angle);
    pbio_actuation_t after_stop = pb_type_enum_get_value(then, &pb_enum_type_Stop);

    // Call pbio with parsed user/default arguments
    pb_assert(pbio_servo_run_angle(self->srv, speed_arg, angle_arg, after_stop));

    if (mp_obj_is_true(wait)) {
        wait_for_completion(self->srv);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_run_angle_obj, 1, motor_Motor_run_angle);

// pybricks.builtins.Motor.run_target
STATIC mp_obj_t motor_Motor_run_target(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(target_angle),
        PB_ARG_DEFAULT_OBJ(then, pb_Stop_HOLD_obj),
        PB_ARG_DEFAULT_TRUE(wait));

    mp_int_t speed_arg = pb_obj_get_int(speed);
    mp_int_t angle_arg = pb_obj_get_int(target_angle);
    pbio_actuation_t after_stop = pb_type_enum_get_value(then, &pb_enum_type_Stop);

    // Call pbio with parsed user/default arguments
    pb_assert(pbio_servo_run_target(self->srv, speed_arg, angle_arg, after_stop));

    if (mp_obj_is_true(wait)) {
        wait_for_completion(self->srv);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_run_target_obj, 1, motor_Motor_run_target);

// pybricks.builtins.Motor.track_target
STATIC mp_obj_t motor_Motor_track_target(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        motor_Motor_obj_t, self,
        PB_ARG_REQUIRED(target_angle));

    mp_int_t target = pb_obj_get_int(target_angle);
    pb_assert(pbio_servo_track_target(self->srv, target));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(motor_Motor_track_target_obj, 1, motor_Motor_track_target);

// dir(pybricks.builtins.Motor)
STATIC const mp_rom_map_elem_t motor_Motor_locals_dict_table[] = {
    //
    // Methods common to DC motors and encoded motors
    //
    { MP_ROM_QSTR(MP_QSTR_dc), MP_ROM_PTR(&motor_DCMotor_duty_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&motor_DCMotor_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_brake), MP_ROM_PTR(&motor_DCMotor_brake_obj) },
    //
    // Methods specific to encoded motors
    //
    { MP_ROM_QSTR(MP_QSTR_hold), MP_ROM_PTR(&motor_Motor_hold_obj) },
    { MP_ROM_QSTR(MP_QSTR_angle), MP_ROM_PTR(&motor_Motor_angle_obj) },
    { MP_ROM_QSTR(MP_QSTR_speed), MP_ROM_PTR(&motor_Motor_speed_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset_angle), MP_ROM_PTR(&motor_Motor_reset_angle_obj) },
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&motor_Motor_run_obj) },
    { MP_ROM_QSTR(MP_QSTR_run_time), MP_ROM_PTR(&motor_Motor_run_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_run_until_stalled), MP_ROM_PTR(&motor_Motor_run_until_stalled_obj) },
    { MP_ROM_QSTR(MP_QSTR_run_angle), MP_ROM_PTR(&motor_Motor_run_angle_obj) },
    { MP_ROM_QSTR(MP_QSTR_run_target), MP_ROM_PTR(&motor_Motor_run_target_obj) },
    { MP_ROM_QSTR(MP_QSTR_track_target), MP_ROM_PTR(&motor_Motor_track_target_obj) },
    { MP_ROM_QSTR(MP_QSTR_log), MP_ROM_ATTRIBUTE_OFFSET(motor_Motor_obj_t, logger) },
    { MP_ROM_QSTR(MP_QSTR_control), MP_ROM_ATTRIBUTE_OFFSET(motor_Motor_obj_t, control) },
};
MP_DEFINE_CONST_DICT(motor_Motor_locals_dict, motor_Motor_locals_dict_table);

// type(pybricks.builtins.Motor)
const mp_obj_type_t motor_Motor_type = {
    { &mp_type_type },
    .name = MP_QSTR_Motor,
    .print = motor_DCMotor_print,
    .make_new = motor_Motor_make_new,
    .attr = pb_obj_generic_and_offset_attr,
    .locals_dict = (mp_obj_dict_t *)&motor_Motor_locals_dict,
};

#endif // PBDRV_CONFIG_NUM_MOTOR_CONTROLLER
