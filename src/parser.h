#include "yeast.h"

#ifndef PARSER_H
#define PARSER_H

YEAST_DEFUN(make_parser, emacs_value language);
YEAST_DEFUN(parser_p, emacs_value _parser);

#endif /* PARSER_H */
