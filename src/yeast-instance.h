#include "yeast.h"

#ifndef PARSER_H
#define PARSER_H

YEAST_DEFUN(make_instance, emacs_value language);
YEAST_DEFUN(instance_p, emacs_value obj);

YEAST_DEFUN(parse_string, emacs_value _instance, emacs_value _str);

#endif /* PARSER_H */
