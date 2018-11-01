#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "emacs-module.h"
#include "interface.h"

#define GLOBREF(val) env->make_global_ref(env, (val))
#define INTERN(val) env->intern(env, (val))

// We store some global references to emacs objects, mostly symbols,
// so that we don't have to waste time calling intern later on.
emacs_value em_nil, em_t;

// Symbols that are only reachable from within this file.
static emacs_value _defalias, _provide;

void em_init(emacs_env *env)
{
    em_nil = GLOBREF(INTERN("nil"));
    em_t = GLOBREF(INTERN("t"));

    _defalias = GLOBREF(INTERN("defalias"));
    _provide = GLOBREF(INTERN("provide"));
}

/**
 * Call an Emacs function without error checking.
 * @param env The active Emacs environment.
 * @param func The function to call.
 * @param nargs The number of arguments that follow.
 * @return The function return value.
 */
static emacs_value em_funcall(emacs_env *env, emacs_value func, ptrdiff_t nargs, ...)
{
    emacs_value args[nargs];

    va_list vargs;
    va_start(vargs, nargs);
    for (ptrdiff_t i = 0; i < nargs; i++)
        args[i] = va_arg(vargs, emacs_value);
    va_end(vargs);

    return env->funcall(env, func, nargs, args);
}

void em_defun(emacs_env *env, const char *name, emacs_value func)
{
    em_funcall(env, _defalias, 2, INTERN(name), func);
}

void em_provide(emacs_env *env, const char *feature)
{
    em_funcall(env, _provide, 1, INTERN(feature));
}
