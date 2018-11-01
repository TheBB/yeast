#include <stdarg.h>

#include "emacs-module.h"

#ifndef INTERFACE_H
#define INTERFACE_H

extern emacs_value em_nil, em_t;

/**
 * Initialize the libyeast-emacs interface.
 * This function should only be called once.
 */
void em_init(emacs_env *env);

/**
 * Provide a feature to Emacs.
 * @param env The active Emacs environment.
 * @param name Symbol name of the feature to provide.
 */
void em_provide(emacs_env *env, const char *feature);

#endif /* INTERFACE_H */
