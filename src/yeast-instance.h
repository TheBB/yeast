#include "yeast.h"

#ifndef YEAST_INSTANCE_H
#define YEAST_INSTANCE_H

YEAST_DEFUN(make_instance, emacs_value language);
YEAST_DEFUN(instance_p, emacs_value obj);

YEAST_DEFUN(parse, emacs_value _instance);
YEAST_DEFUN(edit, emacs_value _instance, emacs_value _beg, emacs_value _end, emacs_value _len);

#endif /* YEAST_INSTANCE_H */
