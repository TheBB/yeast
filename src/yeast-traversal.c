#include <assert.h>

#include "tree_sitter/runtime.h"

#include "interface.h"
#include "yeast.h"
#include "yeast-traversal.h"

/**
 * Convenience function for returning a new node belonging to
 * the same tree as an existing node.
 */
static emacs_value new_node_from_node(emacs_env *env, yeast_node *node, TSNode new)
{
    if (ts_node_is_null(new))
        return em_nil;
    assert(new.tree == node->tree->tree);
    node->tree->header.refcount++;
    yeast_node *retval = (yeast_node*) malloc(sizeof(yeast_node));
    *retval = (yeast_node) {{YEAST_NODE, 0}, node->tree, new};
    return env->make_user_ptr(env, yeast_finalize, retval);
}

YEAST_DOC(tree_p, "OBJ", "Return non-nil if OBJ is a yeast tree.");
emacs_value yeast_tree_p(emacs_env *env, emacs_value obj)
{
    yeast_type type = yeast_get_type(env, obj);
    return type == YEAST_TREE ? em_t : em_nil;
}

YEAST_DOC(node_p, "OBJ", "Return non-nil if OBJ is a yeast node.");
emacs_value yeast_node_p(emacs_env *env, emacs_value obj)
{
    yeast_type type = yeast_get_type(env, obj);
    return type == YEAST_NODE ? em_t : em_nil;
}

YEAST_DOC(instance_tree, "INSTANCE", "Get the current tree in INSTANCE.");
emacs_value yeast_instance_tree(emacs_env *env, emacs_value _instance)
{
    YEAST_ASSERT_INSTANCE(_instance);
    yeast_instance *instance = YEAST_EXTRACT_INSTANCE(_instance);

    if (!instance->tree) {
        em_signal_error(env, "instance has no tree");
        return em_nil;
    }

    instance->header.refcount++;
    TSTree *tree = ts_tree_copy(instance->tree);
    yeast_tree *retval = (yeast_tree*) malloc(sizeof(yeast_tree));
    *retval = (yeast_tree) {{YEAST_TREE, 1}, instance, tree};
    return env->make_user_ptr(env, yeast_finalize, retval);
}

YEAST_DOC(tree_root, "TREE", "Get the root node of TREE.");
emacs_value yeast_tree_root(emacs_env *env, emacs_value _tree)
{
    YEAST_ASSERT_TREE(_tree);
    yeast_tree *tree = YEAST_EXTRACT_TREE(_tree);

    TSNode node = ts_tree_root_node(tree->tree);
    if (ts_node_is_null(node))
        return em_nil;

    tree->header.refcount++;
    yeast_node *retval = (yeast_node*) malloc(sizeof(yeast_node));
    *retval = (yeast_node) {{YEAST_NODE, 0}, tree, node};
    return env->make_user_ptr(env, yeast_finalize, retval);
}

YEAST_DOC(node_type, "NODE", "Get the type of NODE.");
emacs_value yeast_node_type(emacs_env *env, emacs_value _node)
{
    YEAST_ASSERT_NODE(_node);
    yeast_node *node = YEAST_EXTRACT_NODE(_node);
    const char *type = ts_node_type(node->node);
    return env->intern(env, type);
}

YEAST_DOC(node_child_count, "NODE &optional NAMED",
          "Get the number of children of NODE.\n\n"
          "If NAMED is non-nil, count only the named children.");
emacs_value yeast_node_child_count(emacs_env *env, emacs_value _node, emacs_value _named)
{
    YEAST_ASSERT_NODE(_node);
    yeast_node *node = YEAST_EXTRACT_NODE(_node);
    bool named = YEAST_EXTRACT_BOOLEAN(_named);
    uint32_t retval = named ? ts_node_named_child_count(node->node) : ts_node_child_count(node->node);
    return env->make_integer(env, retval);
}

YEAST_DOC(node_child, "NODE INDEX &optional NAMED",
          "Get the child at INDEX below NODE.\n\n"
          "If NAMED is non-nil, count only the named children.");
emacs_value yeast_node_child(emacs_env *env, emacs_value _node, emacs_value _index, emacs_value _named)
{
    YEAST_ASSERT_NODE(_node);
    YEAST_ASSERT_INTEGER(_index);

    yeast_node *node = YEAST_EXTRACT_NODE(_node);
    uint32_t index = YEAST_EXTRACT_INTEGER(_index);
    bool named = YEAST_EXTRACT_BOOLEAN(_named);

    TSNode child = named ? ts_node_named_child(node->node, index) : ts_node_child(node->node, index);
    return new_node_from_node(env, node, child);
}