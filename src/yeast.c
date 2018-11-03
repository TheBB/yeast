#include "emacs-module.h"
#include "tree_sitter/runtime.h"

#include <stdio.h>

#include "interface.h"
#include "yeast-instance.h"
#include "yeast-traversal.h"
#include "yeast.h"

yeast_type yeast_get_type(emacs_env *env, emacs_value _obj)
{
    if (!em_user_ptrp(env, _obj))
        return YEAST_UNKNOWN;
    yeast_header *obj = (yeast_header*) env->get_user_ptr(env, _obj);
    return obj->type;
}

bool yeast_assert_type(emacs_env *env, emacs_value obj, yeast_type type, emacs_value predicate)
{
    if (type == yeast_get_type(env, obj))
        return true;
    em_signal_wrong_type(env, predicate, obj);
    return false;
}

void yeast_finalize(void *_obj)
{
    yeast_header *header = (yeast_header*) _obj;

    if (header->type == YEAST_INSTANCE) {
        header->refcount--;
        if (header->refcount <= 0) {
            yeast_instance *instance = (yeast_instance*) _obj;
            if (instance->tree)
                ts_tree_delete(instance->tree);
            ts_parser_delete(instance->parser);
            free(instance);
        }
    }
    else if (header->type == YEAST_TREE) {
        header->refcount--;
        if (header->refcount <= 0) {
            yeast_tree *tree = (yeast_tree*) _obj;
            ts_tree_delete(tree->tree);
            yeast_instance *instance = tree->instance;
            free(tree);
            yeast_finalize(instance);
        }
    }
    else if (header->type == YEAST_NODE) {
        // Nodes are not reference counted
        yeast_node *node = (yeast_node*) _obj;
        yeast_finalize(node->tree);
        free(node);
    }
}

typedef emacs_value (*func_1)(emacs_env*, emacs_value);
typedef emacs_value (*func_2)(emacs_env*, emacs_value, emacs_value);
typedef emacs_value (*func_3)(emacs_env*, emacs_value, emacs_value, emacs_value);

#define GET_SAFE(arglist, nargs, index) ((index) < (nargs) ? (arglist)[(index)] : em_nil)

static emacs_value yeast_dispatch_1(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_1 func = (func_1) data;
    return func(env, GET_SAFE(args, nargs, 0));
}

static emacs_value yeast_dispatch_2(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_2 func = (func_2) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1));
}

static emacs_value yeast_dispatch_3(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_3 func = (func_3) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2));
}

#define DEFUN(ename, cname, min_nargs, max_nargs)                       \
    em_defun(env, (ename),                                              \
             env->make_function(                                        \
                 env, (min_nargs), (max_nargs),                         \
                 yeast_dispatch_##max_nargs,                            \
                 yeast_##cname##__doc,                                  \
                 yeast_##cname))

void yeast_init(emacs_env *env)
{
    DEFUN("yeast-instance-p", instance_p, 1, 1);
    DEFUN("yeast-tree-p", tree_p, 1, 1);
    DEFUN("yeast-node-p", node_p, 1, 1);
    DEFUN("yeast-node-eq", node_eq, 2, 2);

    DEFUN("yeast--make-instance", make_instance, 1, 1);
    DEFUN("yeast--parse", parse, 1, 1);

    DEFUN("yeast--instance-tree", instance_tree, 1, 1);
    DEFUN("yeast--tree-root", tree_root, 1, 1);
    DEFUN("yeast--node-type", node_type, 1, 1);
    DEFUN("yeast--node-child-count", node_child_count, 1, 2);
    DEFUN("yeast--node-child", node_child, 2, 3);
    DEFUN("yeast--node-start-byte", node_start_byte, 1, 1);
    DEFUN("yeast--node-end-byte", node_end_byte, 1, 1);
    DEFUN("yeast--node-byte-range", node_byte_range, 1, 1);
    DEFUN("yeast--node-child-for-byte", node_child_for_byte, 2, 3);
}
