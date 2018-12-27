
/*
 *
 * Copyright (C) NGINX, Inc.
 */

#include <njs_core.h>
#include <string.h>

njs_ret_t
njs_symbol_to_string(njs_vm_t *vm, njs_value_t *string,
    const njs_value_t *symbol)
{
    static const njs_value_t  unnamed = njs_string("Symbol()");
    u_char                    *p, *start;
    size_t                    size, length;
    nxt_str_t                 name, tag;
    njs_value_t               value;

    njs_string_get(symbol, &name);

    if (name.length == 0) {
        *string = unnamed;

        return NXT_OK;
    }

    njs_string_get(&unnamed, &tag);

    // ???
    size = name.length + tag.length;
    length = size;

    start = njs_string_alloc(vm, &value, size, length);
    if (nxt_slow_path(start == NULL)) {
        return NXT_ERROR;
    }

    p = start;

    p = memcpy(p, tag.start, tag.length - 1);
    p += tag.length - 1;
    p = memcpy(p, name.start, name.length);
    p += name.length;
    *p = ')';

    *string = value;

    return NXT_OK;
}


njs_ret_t
njs_symbol_constructor(njs_vm_t *vm, njs_value_t *args, nxt_uint_t nargs,
    njs_index_t unused)
{
    // TODO: dummy implementation
    njs_value_t  *value;

    if (vm->top_frame->ctor) {

        njs_type_error(vm, "Symbol is not a constructor");

        return NXT_ERROR;
    }

    if (nargs == 1) {
        value = &vm->retval;

        value->short_string.size = 0;
        value->short_string.length = 0;

    } else {
        value = &args[1];
        vm->retval = *value;
    }

    // XXX
    //vm->retval.data.truth = 1;
    vm->retval.type = NJS_SYMBOL;

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


static njs_ret_t
njs_symbol_prototype_to_string(njs_vm_t *vm, njs_value_t *args,
    nxt_uint_t nargs, njs_index_t unused)
{
    nxt_int_t  ret;

    ret = njs_symbol_prototype_value_of(vm, args, nargs, unused);
    if (nxt_slow_path(ret != NXT_OK)) {
        return ret;
    }

    return njs_symbol_to_string(vm, &vm->retval, &vm->retval);
}


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
