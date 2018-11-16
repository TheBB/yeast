#include "tree_sitter/runtime.h"

#include "interface.h"
#include "yeast.h"
#include "yeast-instance.h"

#define BUFSIZE 4092

TSLanguage *tree_sitter_bash();
TSLanguage *tree_sitter_c();
TSLanguage *tree_sitter_cpp();
TSLanguage *tree_sitter_css();
TSLanguage *tree_sitter_go();
TSLanguage *tree_sitter_html();
TSLanguage *tree_sitter_javascript();
TSLanguage *tree_sitter_json();
TSLanguage *tree_sitter_ocaml();
TSLanguage *tree_sitter_php();
TSLanguage *tree_sitter_python();
TSLanguage *tree_sitter_ruby();
TSLanguage *tree_sitter_rust();
TSLanguage *tree_sitter_typescript();

YEAST_DOC(make_instance, "LANGUAGE", "Make a new yeast instance for the given LANGUAGE.");
emacs_value yeast_make_instance(emacs_env *env, emacs_value language)
{
    YEAST_ASSERT_SYMBOL(language);
    TSParser *parser = ts_parser_new();

    if (env->eq(env, language, em_bash))
        ts_parser_set_language(parser, tree_sitter_bash());
    else if (env->eq(env, language, em_c))
        ts_parser_set_language(parser, tree_sitter_c());
    else if (env->eq(env, language, em_cpp))
        ts_parser_set_language(parser, tree_sitter_cpp());
    else if (env->eq(env, language, em_css))
        ts_parser_set_language(parser, tree_sitter_css());
    else if (env->eq(env, language, em_go))
        ts_parser_set_language(parser, tree_sitter_go());
    else if (env->eq(env, language, em_html))
        ts_parser_set_language(parser, tree_sitter_html());
    else if (env->eq(env, language, em_javascript))
        ts_parser_set_language(parser, tree_sitter_javascript());
    else if (env->eq(env, language, em_json))
        ts_parser_set_language(parser, tree_sitter_json());
    else if (env->eq(env, language, em_ocaml))
        ts_parser_set_language(parser, tree_sitter_ocaml());
    else if (env->eq(env, language, em_php))
        ts_parser_set_language(parser, tree_sitter_php());
    else if (env->eq(env, language, em_python))
        ts_parser_set_language(parser, tree_sitter_python());
    else if (env->eq(env, language, em_ruby))
        ts_parser_set_language(parser, tree_sitter_ruby());
    else if (env->eq(env, language, em_rust))
        ts_parser_set_language(parser, tree_sitter_rust());
    else if (env->eq(env, language, em_typescript))
        ts_parser_set_language(parser, tree_sitter_typescript());
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

typedef struct {
    emacs_env *env;
    uint32_t size;
    bool success;
    char buffer[BUFSIZE + 1];
} read_payload;

static const char *read(void *_payload, uint32_t offset, TSPoint position, uint32_t *bytes_read)
{
    read_payload *payload = (read_payload*)_payload;

    // Calculate how many bytes to read
    *bytes_read = payload->size - offset;
    if (*bytes_read > BUFSIZE)
        *bytes_read = BUFSIZE;
    if (*bytes_read <= 0) {
        *bytes_read = 0;
        return "";
    }

    bool retval = em_buffer_contents(payload->env, offset, *bytes_read, &payload->buffer[0]);
    if (!retval) {
        *bytes_read = 0;
        payload->success = false;
        return "";
    }

    return &payload->buffer[0];
}

static emacs_value reparse(emacs_env *env, yeast_instance *instance)
{
    read_payload payload;
    payload.env = env;
    payload.size = em_buffer_size(env);
    payload.success = true;

    TSInput input = {&payload, read, TSInputEncodingUTF8};
    TSTree *new_tree = ts_parser_parse(instance->parser, instance->tree, input);

    emacs_value retval = payload.success ? em_t : em_nil;

    if (instance->tree)
        ts_tree_delete(instance->tree);
    instance->tree = new_tree;

    return retval;
}

YEAST_DOC(parse, "INSTANCE",
          "Parse the current buffer, overriding the current tree in INSTANCE.\n\n"
          "Return non-nil if successful.");
emacs_value yeast_parse(emacs_env *env, emacs_value _instance)
{
    YEAST_ASSERT_INSTANCE(_instance);
    yeast_instance *instance = YEAST_EXTRACT_INSTANCE(_instance);
    return reparse(env, instance);
}

YEAST_DOC(edit, "INSTANCE BEG END LEN",
          "Re-parse the current buffer, overriding the current tree in INSTANCE.\n\n"
          "BEG END and LEN are zero-based byte indexes of the recent change,\n"
          "corresponding to `after-change-functions'.");
emacs_value yeast_edit(
    emacs_env *env, emacs_value _instance,
    emacs_value _beg, emacs_value _end, emacs_value _len)
{
    YEAST_ASSERT_INSTANCE(_instance);
    YEAST_ASSERT_INTEGER(_beg);
    YEAST_ASSERT_INTEGER(_end);
    YEAST_ASSERT_INTEGER(_len);

    yeast_instance *instance = YEAST_EXTRACT_INSTANCE(_instance);
    uint32_t start = YEAST_EXTRACT_INTEGER(_beg);
    uint32_t old_end = start + YEAST_EXTRACT_INTEGER(_len);
    uint32_t new_end = YEAST_EXTRACT_INTEGER(_end);

    TSInputEdit edit = {start, old_end, new_end, {0, 0}, {0, 0}};
    ts_tree_edit(instance->tree, &edit);

    return reparse(env, instance);
}
