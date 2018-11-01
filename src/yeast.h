#include "emacs-module.h"

#ifndef YEAST_H
#define YEAST_H

/**
 * Macro that defines a docstring for a function.
 * @param name The function name (without yeast_ prefix).
 * @param args The argument list as visible from Emacs (without parens).
 * @param docstring The rest of the documentation.
 */
#define YEAST_DOC(name, args, docstring)                                \
    const char *yeast_##name##__doc = (docstring "\n\n(fn " args ")")

/**
 * Macro that declares a function and its docstring variable.
 * @param name The function name (without egit_ prefix)
 * @param ... The function arguments (without emacs_env)
 */
#define YEAST_DEFUN(name, ...)                                  \
    extern const char *yeast_##name##__doc;                     \
    emacs_value yeast_##name(emacs_env *env, __VA_ARGS__)       \

/**
 * Assert that VAL is a symbol, signal an error and return otherwise.
 */
#define YEAST_ASSERT_SYMBOL(val)                                        \
    do { if (!em_assert_type(env, em_symbolp, (val))) return em_nil; } while (0)

/**
 * Define functions visible to Emacs.
 * This function only needs to be called once.
 * @param env The active Emacs environment.
 */
void yeast_init(emacs_env *env);

#endif /* YEAST_H */
