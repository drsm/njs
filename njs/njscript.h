
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) NGINX, Inc.
 *
 * njsScript public header.
 */

#ifndef _NJSCRIPT_H_INCLUDED_
#define _NJSCRIPT_H_INCLUDED_

#define NJS_VERSION                 "0.1.15"


typedef intptr_t                    njs_ret_t;
typedef uintptr_t                   njs_index_t;
typedef struct njs_vm_s             njs_vm_t;
typedef union  njs_value_s          njs_value_t;
typedef struct njs_extern_s         njs_extern_t;
typedef struct njs_function_s       njs_function_t;
typedef struct njs_vm_shared_s      njs_vm_shared_t;

typedef struct {
    uint64_t                        filler[2];
} njs_opaque_value_t;


/* sizeof(njs_value_t) is 16 bytes. */
#define njs_argument(args, n)                                                 \
    (njs_value_t *) ((u_char *) args + n * 16)


typedef njs_ret_t (*njs_extern_get_t)(njs_vm_t *vm, njs_value_t *value,
    void *obj, uintptr_t data);
typedef njs_ret_t (*njs_extern_set_t)(njs_vm_t *vm, void *obj, uintptr_t data,
    nxt_str_t *value);
typedef njs_ret_t (*njs_extern_find_t)(njs_vm_t *vm, void *obj, uintptr_t data,
    nxt_bool_t delete);
typedef njs_ret_t (*njs_extern_foreach_t)(njs_vm_t *vm, void *obj, void *next);
typedef njs_ret_t (*njs_extern_next_t)(njs_vm_t *vm, njs_value_t *value,
    void *obj, void *next);
typedef njs_ret_t (*njs_extern_method_t)(njs_vm_t *vm, njs_value_t *args,
    nxt_uint_t nargs, njs_index_t unused);


typedef struct njs_external_s       njs_external_t;

struct njs_external_s {
    nxt_str_t                       name;

#define NJS_EXTERN_PROPERTY         0x00
#define NJS_EXTERN_METHOD           0x01
#define NJS_EXTERN_OBJECT           0x80
#define NJS_EXTERN_CASELESS_OBJECT  0x81

    uintptr_t                       type;

    njs_external_t                  *properties;
    nxt_uint_t                      nproperties;

    njs_extern_get_t                get;
    njs_extern_set_t                set;
    njs_extern_find_t               find;

    njs_extern_foreach_t            foreach;
    njs_extern_next_t               next;

    njs_extern_method_t             method;

    uintptr_t                       data;
};


/*
 * NJS and event loops.
 *
 * njs_vm_ops_t callbacks are used to interact with the event loop environment.
 *
 * Functions get an external object as the first argument. The external
 * object is provided as the third argument to njs_vm_clone().
 *
 * The callbacks are expected to return to the VM the unique id of an
 * underlying event.  This id will be passed as the second argument to
 * njs_event_destructor() at the moment the VM wants to destroy it.
 *
 * When an underlying events fires njs_vm_post_event() should be invoked with
 * the value provided as vm_event.
 *
 * The events posted by njs_vm_post_event() are processed as soon as
 * njs_vm_run() is invoked. njs_vm_run() returns NJS_AGAIN until pending events
 * are present.
 */

typedef void *                      njs_vm_event_t;
typedef void *                      njs_host_event_t;
typedef void *                      njs_external_ptr_t;

typedef njs_host_event_t (*njs_set_timer)(njs_external_ptr_t external,
    uint64_t delay, njs_vm_event_t vm_event);
typedef void (*njs_event_destructor)(njs_external_ptr_t external,
    njs_host_event_t event);


typedef struct {
    njs_set_timer                   set_timer;
    njs_event_destructor            clear_timer;
} njs_vm_ops_t;


typedef struct {
    njs_external_ptr_t              external;
    njs_vm_shared_t                 *shared;
    njs_vm_ops_t                    *ops;

    uint8_t                         trailer;         /* 1 bit */
    uint8_t                         accumulative;    /* 1 bit */
    uint8_t                         backtrace;       /* 1 bit */
} njs_vm_opt_t;


typedef struct {
    nxt_str_t                       name;
    uint32_t                        line;
} njs_backtrace_entry_t;


#define NJS_OK                      NXT_OK
#define NJS_ERROR                   NXT_ERROR
#define NJS_AGAIN                   NXT_AGAIN
#define NJS_DECLINED                NXT_DECLINED
#define NJS_DONE                    NXT_DONE


NXT_EXPORT njs_vm_t *njs_vm_create(njs_vm_opt_t *options);
NXT_EXPORT void njs_vm_destroy(njs_vm_t *vm);

NXT_EXPORT nxt_int_t njs_vm_compile(njs_vm_t *vm, u_char **start, u_char *end);
NXT_EXPORT njs_vm_t *njs_vm_clone(njs_vm_t *vm, njs_external_ptr_t external);
NXT_EXPORT nxt_int_t njs_vm_call(njs_vm_t *vm, njs_function_t *function,
    njs_opaque_value_t *args, nxt_uint_t nargs);

NXT_EXPORT nxt_int_t njs_vm_pending(njs_vm_t *vm);
NXT_EXPORT nxt_int_t njs_vm_post_event(njs_vm_t *vm, njs_vm_event_t vm_event);

NXT_EXPORT nxt_int_t njs_vm_run(njs_vm_t *vm);

NXT_EXPORT const njs_extern_t *njs_vm_external_prototype(njs_vm_t *vm,
    njs_external_t *external);
NXT_EXPORT nxt_int_t njs_vm_external_create(njs_vm_t *vm,
        njs_opaque_value_t *value, const njs_extern_t *proto, void *object);
NXT_EXPORT nxt_int_t njs_vm_external_bind(njs_vm_t *vm,
    const nxt_str_t *var_name, njs_opaque_value_t *value);

NXT_EXPORT void njs_disassembler(njs_vm_t *vm);
NXT_EXPORT nxt_array_t *njs_vm_completions(njs_vm_t *vm, nxt_str_t *expression);

NXT_EXPORT njs_function_t *njs_vm_function(njs_vm_t *vm, nxt_str_t *name);
NXT_EXPORT njs_value_t *njs_vm_retval(njs_vm_t *vm);
NXT_EXPORT void njs_vm_retval_set(njs_vm_t *vm, njs_opaque_value_t *value);

NXT_EXPORT u_char * njs_string_alloc(njs_vm_t *vm, njs_value_t *value,
    uint32_t size, uint32_t length);
NXT_EXPORT njs_ret_t njs_string_create(njs_vm_t *vm, njs_value_t *value,
    u_char *start, uint32_t size, uint32_t length);

NXT_EXPORT nxt_int_t njs_value_string_copy(njs_vm_t *vm, nxt_str_t *retval,
    njs_value_t *value, uintptr_t *next);

NXT_EXPORT njs_ret_t njs_vm_value_to_ext_string(njs_vm_t *vm, nxt_str_t *dst,
    const njs_value_t *src, nxt_uint_t handle_exception);
NXT_EXPORT njs_ret_t njs_vm_retval_to_ext_string(njs_vm_t *vm,
        nxt_str_t *retval);

NXT_EXPORT void njs_value_void_set(njs_value_t *value);
NXT_EXPORT void njs_value_boolean_set(njs_value_t *value, int yn);
NXT_EXPORT void njs_value_number_set(njs_value_t *value, double num);
NXT_EXPORT void *njs_value_data(njs_value_t *value);

NXT_EXPORT nxt_int_t njs_value_is_void(njs_value_t *value);


extern const nxt_mem_proto_t  njs_vm_mem_cache_pool_proto;

#endif /* _NJSCRIPT_H_INCLUDED_ */
