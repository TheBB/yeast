#include "emacs-module.h"
#include "tree_sitter/runtime.h"

#include "interface.h"
#include "parser.h"
#include "yeast.h"

// Hash table of stored objects
yeast_object *object_store = NULL;

yeast_type yeast_get_type(emacs_env *env, emacs_value _obj)
{
    if (!em_user_ptrp(env, _obj))
        return YEAST_UNKNOWN;
    yeast_object *obj = (yeast_object*)env->get_user_ptr(env, _obj);
    return obj->type;
}

/**
 * Finalizer for user pointers with no children objects.
 * This frees the wrapper struct, its wrapped object and decreses the reference count on parent
 * objects, if any, potentially causing them to be freed.
 * This function is suitable to use as a finalizer for Emacs user pointers.
 */
static void yeast_finalize(void *_obj)
{
    yeast_object *obj = (yeast_object*) _obj;

    switch(obj->type) {
    case YEAST_PARSER:
        ts_parser_delete(obj->ptr);
        break;
    }

    free(obj);
}

emacs_value yeast_wrap(emacs_env *env, yeast_type type, void *ptr)
{
    yeast_object *wrapper = (yeast_object*) malloc(sizeof(yeast_object));
    wrapper->type = type;
    wrapper->ptr = ptr;

    return env->make_user_ptr(env, yeast_finalize, wrapper);
}

typedef emacs_value (*func_1)(emacs_env*, emacs_value);

#define GET_SAFE(arglist, nargs, index) ((index) < (nargs) ? (arglist)[(index)] : em_nil)

static emacs_value yeast_dispatch_1(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_1 func = (func_1) data;
    return func(env, GET_SAFE(args, nargs, 0));
}

#define DEFUN(ename, cname, min_nargs, max_nargs)                       \
    em_defun(env, (ename),                                              \
             env->make_function(                                        \
                 env, (min_nargs), (max_nargs),                         \
                 yeast_dispatch_##max_nargs,                            \
                 yeast_##cname##__doc,                                  \
                 yeast_##cname))

void yeast_init(emacs_env *env)
{
    DEFUN("yeast-make-parser", make_parser, 1, 1);
    DEFUN("yeast-parser-p", parser_p, 1, 1);
}
