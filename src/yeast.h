#include "emacs-module.h"
#include "uthash.h"

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
 * Enum used to distinguish between various types of objects exposed.
 */
typedef enum {
    YEAST_UNKNOWN,
    YEAST_PARSER
} yeast_type;

/**
 * Hashable wrapper structure for a yeast object.
 */
typedef struct {
    UT_hash_handle hh;          /**< For internal use by the hash table. */
    yeast_type type;            /**< Type of object stored. */
    ptrdiff_t refcount;         /**< Reference count. */
    void *ptr;                  /**< Data pointer. */
} yeast_object;

/**
 * Return the yeast object type stored by en Emacs value.
 * @param env The active Emacs environment.
 * @param _obj The value to check.
 * @return The object type, or YEAST_UNKNOWN if not known.
 */
yeast_type yeast_get_type(emacs_env *env, emacs_value _obj);

/**
 * Wrap a structure in an emacs_value.
 * @param env The active Emacs environment.
 * @param obj The type of the object.
 * @param ptr The pointer to store.
 * @return The Emacs value.
 */
emacs_value yeast_wrap(emacs_env *env, yeast_type type, void* ptr);

/**
 * Define functions visible to Emacs.
 * This function only needs to be called once.
 * @param env The active Emacs environment.
 */
void yeast_init(emacs_env *env);

#endif /* YEAST_H */
