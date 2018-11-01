#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "emacs-module.h"

#include "interface.h"
#include "yeast.h"

int plugin_is_GPL_compatible;

static bool initialized = false;

/**
 * Initialize the libegit2 module.
 * This is only done once, no matter how many times it's called.
 */
int emacs_module_init(struct emacs_runtime *ert)
{
    if (initialized)
        return 0;

    emacs_env *env = ert->get_environment(ert);

    // Initialize our own interface to Emacs
    em_init(env);

    // Define all lisp-callable functions
    yeast_init(env);

    em_provide(env, "libyeast");
    initialized = true;
    return 0;
}
