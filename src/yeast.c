#include "emacs-module.h"

#include "interface.h"
#include "parser.h"
#include "yeast.h"

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
}
