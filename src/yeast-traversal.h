#include "yeast.h"

#ifndef YEAST_TRAVERSAL_H
#define YEAST_TRAVERSAL_H

YEAST_DEFUN(tree_p, emacs_value obj);
YEAST_DEFUN(node_p, emacs_value obj);
YEAST_DEFUN(node_eq, emacs_value _obj1, emacs_value _obj2);

YEAST_DEFUN(instance_tree, emacs_value _instance);
YEAST_DEFUN(tree_root, emacs_value _tree);

YEAST_DEFUN(node_type, emacs_value _node);
YEAST_DEFUN(node_child_count, emacs_value _node, emacs_value _anon);
YEAST_DEFUN(node_child, emacs_value _node, emacs_value _index, emacs_value _anon);
YEAST_DEFUN(node_start_byte, emacs_value _node);
YEAST_DEFUN(node_end_byte, emacs_value _node);
YEAST_DEFUN(node_byte_range, emacs_value _node);
YEAST_DEFUN(node_child_for_byte, emacs_value _node, emacs_value _byte, emacs_value _anon);

YEAST_DEFUN(next_sibling, emacs_value _node, emacs_value _anon);
YEAST_DEFUN(prev_sibling, emacs_value _node, emacs_value _anon);
YEAST_DEFUN(parent, emacs_value _node);

#endif /* YEAST_TRAVERSAL_H */
