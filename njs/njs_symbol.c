
/*
 *
 * Copyright (C) NGINX, Inc.
 */

#include <njs_core.h>
#include <string.h>

njs_ret_t
njs_symbol_constructor(njs_vm_t *vm, njs_value_t *args, nxt_uint_t nargs,
    njs_index_t unused)
{
    //njs_object_t       *object;
    //const njs_value_t  *value;

    //if (nargs == 1) {
    //     value = &njs_string_empty;

    // } else {
    //     value = &args[1];
    // }

    if (vm->top_frame->ctor) {
        njs_type_error(vm, "Symbol is not a constructor");
        return NXT_ERROR;
        /*
        object = njs_object_value_alloc(vm, value, value->type);
        if (nxt_slow_path(object == NULL)) {
            return NXT_ERROR;
        }

        vm->retval.data.u.object = object;
        vm->retval.type = NJS_OBJECT_SYMBOL;
        vm->retval.data.truth = 1;
        */
    } else {
        vm->retval.type = NJS_SYMBOL;
        vm->retval.data.truth = 1;
    }

    return NXT_OK;
}


static const njs_object_prop_t  njs_symbol_constructor_properties[] =
{
    /* Symbol.name == "Symbol". */
    {
        .type = NJS_PROPERTY,
        .name = njs_string("name"),
        .value = njs_string("Symbol"),
    },

    /* Symbol.length == 0. */
    {
        .type = NJS_PROPERTY,
        .name = njs_string("length"),
        .value = njs_value(NJS_NUMBER, 1, 0.0),
    },

    /* Symbol.prototype. */
    {
        .type = NJS_PROPERTY_HANDLER,
        .name = njs_string("prototype"),
        .value = njs_prop_handler(njs_object_prototype_create),
    },
};


const njs_object_init_t  njs_symbol_constructor_init = {
    nxt_string("Symbol"),
    njs_symbol_constructor_properties,
    nxt_nitems(njs_symbol_constructor_properties),
};


static njs_ret_t
njs_symbol_prototype_value_of(njs_vm_t *vm, njs_value_t *args, nxt_uint_t nargs,
    njs_index_t unused)
{
    njs_value_t  *value;

    value = &args[0];

    if (value->type != NJS_SYMBOL) {

        if (value->type == NJS_OBJECT_SYMBOL) {
            value = &value->data.u.object_value->value;

        } else {
            njs_type_error(vm, "unexpected value type:%s",
                           njs_type_string(value->type));
            return NXT_ERROR;
        }
    }

    vm->retval = *value;

    return NXT_OK;
}


/*
 * String.toString([encoding]).
 * Returns the string as is if no additional argument is provided,
 * otherwise converts a byte string into an encoded string: hex, base64,
 * base64url.
 */

static njs_ret_t
njs_symbol_prototype_to_string(njs_vm_t *vm, njs_value_t *args,
    nxt_uint_t nargs, njs_index_t unused)
{
    nxt_int_t          ret;
    nxt_str_t          enc, str;
    njs_value_t        value;
    njs_string_prop_t  string;

    ret = njs_symbol_prototype_value_of(vm, args, nargs, unused);
    if (nxt_slow_path(ret != NXT_OK)) {
        return ret;
    }

    if (nargs < 2) {
        return NJS_OK;
    }

    if (nxt_slow_path(!njs_is_string(&args[1]))) {
        njs_type_error(vm, "encoding must be a string");
        return NJS_ERROR;
    }

    value = vm->retval;

    (void) njs_string_prop(&string, &value);

    if (nxt_slow_path(string.length != 0)) {
        njs_type_error(vm, "argument must be a byte string");
        return NJS_ERROR;
    }

    njs_string_get(&args[1], &enc);

    str.length = string.size;
    str.start = string.start;

    if (enc.length == 3 && memcmp(enc.start, "hex", 3) == 0) {
        return njs_string_hex(vm, &vm->retval, &str);

    } else if (enc.length == 6 && memcmp(enc.start, "base64", 6) == 0) {
        return njs_string_base64(vm, &vm->retval, &str);

    } else if (enc.length == 9 && memcmp(enc.start, "base64url", 9) == 0) {
        return njs_string_base64url(vm, &vm->retval, &str);
    }

    njs_type_error(vm, "Unknown encoding: '%.*s'", (int) enc.length, enc.start);

    return NJS_ERROR;
}

/*
njs_ret_t
njs_primitive_value_to_string(njs_vm_t *vm, njs_value_t *dst,
    const njs_value_t *src)
{
    const njs_value_t  *value;

    switch (src->type) {

    case NJS_NULL:
        value = &njs_string_null;
        break;

    case NJS_VOID:
        value = &njs_string_void;
        break;

    case NJS_BOOLEAN:
        value = njs_is_true(src) ? &njs_string_true : &njs_string_false;
        break;

    case NJS_NUMBER:
        return njs_number_to_string(vm, dst, src);

    case NJS_STRING:
        value = src;
        break;

    default:
        return NXT_ERROR;
    }

    *dst = *value;

    return NXT_OK;
}
*/
static const njs_object_prop_t  njs_symbol_prototype_properties[] =
{
    {
        .type = NJS_PROPERTY_HANDLER,
        .name = njs_string("__proto__"),
        .value = njs_prop_handler(njs_primitive_prototype_get_proto),
    },

    {
        .type = NJS_METHOD,
        .name = njs_string("valueOf"),
        .value = njs_native_function(njs_symbol_prototype_value_of, 0, 0),
    },

    {
        .type = NJS_METHOD,
        .name = njs_string("toString"),
        .value = njs_native_function(njs_symbol_prototype_to_string, 0, 0),
    },

};


const njs_object_init_t  njs_symbol_prototype_init = {
    nxt_string("Symbol"),
    njs_symbol_prototype_properties,
    nxt_nitems(njs_symbol_prototype_properties),
};
