#include "emacs-module.h"

#include "interface.h"

int plugin_is_GPL_compatible;

static bool initialized = false;

/**
 * Initialize the yeast module.
 * This is only done once, no matter how many times it's called.
 */
int emacs_module_init(struct emacs_runtime *ert)
{
    if (initialized)
        return 0;

    emacs_env *env = ert->get_environment(ert);

    // Initialize our own interface to Emacs
    em_init(env);

    em_provide(env, "libyeast");
    initialized = true;
    return 0;
}
