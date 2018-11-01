#include "tree_sitter/runtime.h"

#include "interface.h"
#include "yeast.h"
#include "parser.h"

TSLanguage *tree_sitter_json();

YEAST_DOC(make_parser, "LANGUAGE", "Make a new parser object for the given LANGUAGE.");
emacs_value yeast_make_parser(emacs_env *env, emacs_value language)
{
    YEAST_ASSERT_SYMBOL(language);
    TSParser *parser = ts_parser_new();

    if (env->eq(env, language, em_json))
        ts_parser_set_language(parser, tree_sitter_json());
    else {
        ts_parser_delete(parser);
        env->non_local_exit_signal(env, em_unknown_language, em_cons(env, language, em_nil));
        return em_nil;
    }

    return yeast_wrap(env, YEAST_PARSER, parser);
}

YEAST_DOC(parser_p, "PARSER", "Return non-nil if PARSER is a parser.");
emacs_value yeast_parser_p(emacs_env *env, emacs_value obj)
{
    yeast_type type = yeast_get_type(env, obj);
    return type == YEAST_PARSER ? em_t : em_nil;
}
