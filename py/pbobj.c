// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2020 The Pybricks Authors

#include <pbio/error.h>

#include <fixmath.h>

#include "pberror.h"
#include "pbobj.h"

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/objtype.h"
#include "py/runtime.h"

void pb_import_module(qstr name) {
    mp_store_global(name, mp_import_name(name, mp_const_none, MP_OBJ_NEW_SMALL_INT(0)));
}

void pb_from_module_import_all(qstr name) {
    mp_import_all(mp_import_name(name, mp_const_none, MP_OBJ_NEW_SMALL_INT(0)));
}

#if MICROPY_PY_BUILTINS_FLOAT
mp_int_t pb_obj_get_int(mp_obj_t arg) {
    if (mp_obj_is_float(arg)) {
        return (mp_int_t)mp_obj_get_float(arg);
    }
    return mp_obj_get_int(arg);
}
#endif

mp_obj_t pb_obj_new_fraction(int32_t numerator, int32_t denominator) {
    #if MICROPY_PY_BUILTINS_FLOAT
    return mp_obj_new_float(((mp_float_t)numerator) / ((mp_float_t)denominator));
    #else
    return mp_obj_new_int(numerator / denominator);
    #endif
}

fix16_t pb_obj_get_fix16(mp_obj_t arg) {
    #if MICROPY_PY_BUILTINS_FLOAT
    if (mp_obj_is_float(arg)) {
        return fix16_from_float((float)mp_obj_get_float(arg));
    }
    #endif
    return fix16_from_int(mp_obj_get_int(arg));
}

mp_int_t pb_obj_get_default_int(mp_obj_t obj, mp_int_t default_val) {
    return obj == mp_const_none ? default_val : pb_obj_get_int(obj);
}

mp_obj_t pb_obj_get_base_class_obj(mp_obj_t obj, const mp_obj_type_t *type) {

    // If it equals the base type then return as is
    if (mp_obj_is_type(obj, type)) {
        return obj;
    }
    // If it is an instance of a derived class, get base instance
    if (mp_obj_is_obj(obj) && mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t *)MP_OBJ_TO_PTR(obj))->type), type)) {
        return ((mp_obj_instance_t *)MP_OBJ_TO_PTR(obj))->subobj[0];
    }
    // On failure, say why we could not do it
    pb_assert_type(obj, type);
    return MP_OBJ_NULL;
}

const mp_obj_type_t mp_type_offset = {
    { &mp_type_type },
    .name = MP_QSTR_offset,
};

void pb_obj_generic_and_offset_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    const mp_obj_type_t *type = mp_obj_get_type(self_in);
    if (dest[0] == MP_OBJ_NULL && type->locals_dict != NULL) {
        assert(type->locals_dict->base.type == &mp_type_dict);
        mp_map_elem_t *elem = mp_map_lookup(&type->locals_dict->map, MP_OBJ_NEW_QSTR(attr), MP_MAP_LOOKUP);
        if (elem != NULL && MP_OBJ_IS_SMALL_INT(elem->value)) {
            // Get offset to find address of the requested attribute
            size_t offset = MP_OBJ_SMALL_INT_VALUE(elem->value);
            dest[0] = *(mp_obj_t *)((char *)MP_OBJ_TO_PTR(self_in) + offset);
            return;
        }
    }
    dest[1] = MP_OBJ_SENTINEL;
}
