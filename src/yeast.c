#include "emacs-module.h"
#include "tree_sitter/runtime.h"

#include <stdio.h>

#include "interface.h"
#include "yeast-instance.h"
#include "yeast.h"

yeast_type yeast_get_type(emacs_env *env, emacs_value _obj)
{
    if (!em_user_ptrp(env, _obj))
        return YEAST_UNKNOWN;
    yeast_header *obj = (yeast_header*) env->get_user_ptr(env, _obj);
    return obj->type;
}

void yeast_finalize(void *_obj)
{
    yeast_header *header = (yeast_header*) _obj;

    if (header->type == YEAST_INSTANCE) {
        header->refcount--;
        if (header->refcount <= 0) {
            yeast_instance *instance = (yeast_instance*) _obj;
            if (instance->tree)
                ts_tree_delete(instance->tree);
            ts_parser_delete(instance->parser);
            free(instance);
        }
    }
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
    DEFUN("yeast--make-instance", make_instance, 1, 1);
    DEFUN("yeast-instance-p", instance_p, 1, 1);
}
