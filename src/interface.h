#include <stdarg.h>

#include "emacs-module.h"

#ifndef INTERFACE_H
#define INTERFACE_H

extern emacs_value em_nil, em_t;
extern emacs_value em_stringp, em_symbolp;
extern emacs_value em_yeast_instance_p;

extern emacs_value em_unknown_language;

extern emacs_value em_json, em_python;

/**
 * Initialize the libyeast-emacs interface.
 * This function should only be called once.
 */
void em_init(emacs_env *env);

/**
 * Signal a wrong-type-argument error if PREDICATE does not apply to ARG.
 * @param env The active Emacs environment.
 * @param predicate The predicate.
 * @param arg The argument.
 * @return True iff an error was signaled.
 */
bool em_assert_type(emacs_env *env, emacs_value predicate, emacs_value arg);

/**
 * Signal a wrong-type-argument error.
 * @param env The active Emacs environment.
 * @param expected Symbol describing the expected type.
 * @param actual Emacs value that does not have the expected type.
 */
void em_signal_wrong_type(emacs_env *env, emacs_value expected, emacs_value actual);

/**
 * Return a string from an emacs_value.
 * Caller is responsible for ensuring that the value is a string, and to free the returned pointer.
 * @param env The active Emacs environment.
 * @param arg Emacs value representing a string.
 * @return The string (owned pointer).
 */
char *em_get_string(emacs_env *env, emacs_value arg);

/**
 * Call (cons car cdr) in Emacs.
 * @param env The active Emacs environment.
 * @param car The car.
 * @param cdr The cdr.
 * @return The cons cell.
 */
emacs_value em_cons(emacs_env *env, emacs_value car, emacs_value cdr);

/**
 * Define a function in Emacs, using defalias.
 * @param env The active Emacs environment.
 * @param name Symbol name of the desired function.
 * @param func Function to bind.
 */
void em_defun(emacs_env *env, const char *name, emacs_value func);

/**
 * Call (buffer-size) in Emacs.
 */
uint32_t em_buffer_size(emacs_env *env);

/**
 * Copy Emacs buffer contents to a buffer.
 * Caller is responsible that the Emacs buffer contains as many characters as asked for,
 * and that the buffer can hold that many.
 * Note: the Emacs buffer should probably be in unibyte mode!
 * @param env The active Emacs environment.
 * @param offset The starting offset to read from.
 * @param nchars The number of characters to read.
 * @param buffer The buffer to write into.
 * @return True if ok, false if not ok.
 */
bool em_buffer_contents(emacs_env *env, uint32_t offset, uint32_t nchars, char *buffer);

/**
 * Provide a feature to Emacs.
 * @param env The active Emacs environment.
 * @param name Symbol name of the feature to provide.
 */
void em_provide(emacs_env *env, const char *feature);

/**
 * Check if a value is a user pointer.
 * @param env The active Emacs environment.
 * @param val Value to check.
 * @return True iff val is a user pointer.
 */
bool em_user_ptrp(emacs_env *env, emacs_value val);

#endif /* INTERFACE_H */
