#include "tree_sitter/runtime.h"

#include "interface.h"
#include "yeast.h"
#include "yeast-instance.h"

TSLanguage *tree_sitter_json();
TSLanguage *tree_sitter_python();

YEAST_DOC(make_instance, "LANGUAGE", "Make a new yeast instance for the given LANGUAGE.");
emacs_value yeast_make_instance(emacs_env *env, emacs_value language)
{
    YEAST_ASSERT_SYMBOL(language);
    TSParser *parser = ts_parser_new();

    if (env->eq(env, language, em_json))
        ts_parser_set_language(parser, tree_sitter_json());
    else if (env->eq(env, language, em_python))
        ts_parser_set_language(parser, tree_sitter_python());
    else {
        ts_parser_delete(parser);
        env->non_local_exit_signal(env, em_unknown_language, em_cons(env, language, em_nil));
        return em_nil;
    }

    yeast_instance *retval = (yeast_instance*) malloc(sizeof(yeast_instance));
    *retval = (yeast_instance) {{YEAST_INSTANCE, 1}, parser, NULL};
    return env->make_user_ptr(env, yeast_finalize, retval);
}

YEAST_DOC(instance_p, "OBJ", "Return non-nil if OBJ is a yeast instance.");
emacs_value yeast_instance_p(emacs_env *env, emacs_value obj)
{
    yeast_type type = yeast_get_type(env, obj);
    return type == YEAST_INSTANCE ? em_t : em_nil;
}
